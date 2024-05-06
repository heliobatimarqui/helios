/*---------------------------------------------------------------------------------
MIT License

Copyright (c) 2024 Helio Nunes Santos

        Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---------------------------------------------------------------------------------*/

#include "sys/virtualmemory/memmap.hpp"
#include "include/symbols.hpp"
#include "sys/mem.hpp"
#include "sys/print.hpp"
#include "sys/virtualmemory/common.hpp"
#include "sys/virtualmemory/paging.hpp"

namespace hls {

uintptr_t calculate_virtual_ptr_offset(void *vaddress) {
    uintptr_t p = to_uintptr_t(vaddress);
    return p & 0xFFF;
}

size_t get_page_entry_index(PageLevel v, const void *vaddress) {
    size_t vpn_idx = static_cast<size_t>(v);
    uintptr_t idx = to_uintptr_t(vaddress) >> 12;
    return (idx >> (vpn_idx * 9)) & 0x1FF;
}

Result<PageLevel> walk_table(PageTable **table_ptr, const void *vaddress, PageLevel page_level) {
    PageTable *table = *table_ptr;

    if (table == nullptr) {
        return error<PageLevel>(Error::INVALID_PAGE_TABLE);
    }

    // We can't walk the table anymore, thus everything stays the same
    if (page_level == PageLevel::KB_VPN) {
        return error<PageLevel>(Error::VALUE_LIMIT_REACHED);
    }

    uintptr_t vpn = get_page_entry_index(page_level, vaddress);
    auto &entry = table->get_entry(vpn);

    if (!entry.is_valid()) {
        return error<PageLevel>(Error::INVALID_PAGE_ENTRY);
    }

    if (!entry.is_leaf()) {
        *table_ptr = entry.as_table_pointer();
        page_level = next_vpn(page_level);
    }

    return value(page_level);
}

Result<void *> get_physical_address(PageTable *start_table, const void *vaddress) {
    if (start_table == nullptr)
        return error<void *>(Error::INVALID_PAGE_TABLE);

    for (size_t i = 0; i < static_cast<size_t>(PageLevel::LAST_VPN) + 1; ++i) {
        PageLevel v = static_cast<PageLevel>(static_cast<size_t>(PageLevel::LAST_VPN) - i);
        size_t idx = get_page_entry_index(v, vaddress);
        auto &entry = start_table->get_entry(idx);

        if (!entry.is_valid())
            break;

        if (!entry.is_leaf()) {
            start_table = entry.as_table_pointer();
            continue;
        }

        auto p = to_uintptr_t(entry.as_pointer());
        p |= (to_uintptr_t(vaddress) & 0xFFF);

        for (size_t j = 0; j < static_cast<size_t>(v); ++j) {
            size_t offset = get_page_entry_index(static_cast<PageLevel>(j), vaddress);
            p |= offset << (12 + 9 * j);
        }

        return value(to_ptr(p));
    }

    return error<void *>(Error::INVALID_PAGE_ENTRY);
}

// TODO: IMPLEMENT FLAGS FOR MAPED ADDRESSES
Result<const void *> kmmap(const void *paddress, const void *vaddress, PageTable *start_table, PageLevel page_level,
                           bool writable, bool executable, bool accessed, bool dirty) {

    // First lets check if it is already mapped
    auto address_result = get_physical_address(start_table, vaddress);

    if (address_result.is_value()) {
        return error<const void *>(Error::ADDRESS_ALREADY_MAPPED); // If the result is an error, then that
                                                                   // address is not mapped
    }

    // Here we presume we are always using and starting from the highest available
    // paging mode. TODO: change algorithm to accept different starting levels!
    PageTable *table = start_table;
    PageLevel current_page_level = PageLevel::LAST_VPN;

    PageFrameManager &frame_manager = PageFrameManager::instance();

    while (current_page_level != page_level) {
        auto result = walk_table(&table, vaddress, current_page_level);
        if (result.is_error()) {
            switch (result.get_error()) {
            case Error::INVALID_PAGE_ENTRY: {
                size_t entry_idx = get_page_entry_index(current_page_level, vaddress);
                PageEntry &entry = table->get_entry(entry_idx);
                auto frame_result = frame_manager.get_frame();

                if (frame_result.is_error()) {
                    return error<const void *>(frame_result.get_error());
                }

                PageTable *new_table = reinterpret_cast<PageTable *>(frame_result.get_value());
                memset(new_table, 0, PAGE_FRAME_SIZE);
                entry.point_to_table(new_table);
                table = new_table;
                current_page_level = next_vpn(current_page_level);
                continue;
            }
            case Error::INVALID_PAGE_TABLE:
                return error<const void *>(result.get_error());
            default: {
                PANIC("Unhandled error while walking page tables.");
            }
            }
        }
        current_page_level = result.get_value();
    }

    auto &entry = table->get_entry(get_page_entry_index(current_page_level, vaddress));

    auto point_lambda = [](PageEntry &entry, const void *addr, PageLevel page_level) {
        switch (page_level) {
        case PageLevel::TB_VPN:
            entry.point_to_frame<PageLevel::TB_VPN>(reinterpret_cast<const PageFrame<PageLevel::TB_VPN> *>(addr));
            break;
        case PageLevel::GB_VPN:
            entry.point_to_frame<PageLevel::GB_VPN>(reinterpret_cast<const PageFrame<PageLevel::GB_VPN> *>(addr));
            break;
        case PageLevel::MB_VPN:
            entry.point_to_frame<PageLevel::MB_VPN>(reinterpret_cast<const PageFrame<PageLevel::MB_VPN> *>(addr));
            break;
        case PageLevel::KB_VPN:
            entry.point_to_frame<PageLevel::KB_VPN>(reinterpret_cast<const PageFrame<PageLevel::KB_VPN> *>(addr));
            break;
        }
    };

    point_lambda(entry, paddress, page_level);

    if (executable) {
        entry.make_executable(true);
    }
    if (writable) {
        entry.make_writable(true);
    }

    entry.set_accessed(accessed);
    entry.set_dirty(dirty);

    return value(vaddress);
}

void kmunmap(const void *vaddress, PageTable *start_table) {
    kdebug("Unmapping {} on {}.", vaddress, start_table);

    if (vaddress != nullptr || start_table != nullptr) {
        PageTable *table = start_table;
        PageLevel current_page_level = PageLevel::LAST_VPN;

        // Stores all pages used to reach the address
        PageTable *table_path[(size_t)(PageLevel::LAST_VPN) + 1];

        while ((table_path[(size_t)(current_page_level)] = table) && current_page_level != PageLevel::FIRST_VPN) {
            auto result = walk_table(&table, vaddress, current_page_level);
            if (result.is_error()) {
                auto &entry = table->get_entry(get_page_entry_index(current_page_level, vaddress));
                if (entry.is_leaf())
                    break;
                else
                    PANIC("Invalid page table.");
            }

            current_page_level = result.get_value();
        }

        auto &entry = table->get_entry(get_page_entry_index(current_page_level, vaddress));
        entry.erase();

        // Releases a frame if the table is empty
        bool freed_last = false;

        for (size_t i = (size_t)(current_page_level); i < sizeof(table_path) / sizeof(PageTable *); ++i) {
            auto checked_table = table_path[i];
            if (freed_last) {
                size_t j = get_page_entry_index((PageLevel)(i), vaddress);
                auto &entry = checked_table->get_entry(get_page_entry_index((PageLevel)(i), vaddress));
                entry.erase();
            }

            // The root table should always be handled differently.
            if (checked_table->is_empty() && checked_table != start_table) {
                PageFrameManager::instance().release_frame(checked_table);
                freed_last = true;
            } else {
                break;
            }
        }
    }
}

void map_kernel_impl(PageTable *table, void *base_address, bool map) {
    const void *vaddress = reinterpret_cast<const byte *>(base_address);

    auto map_unmap_lambda = [map, table](const void *vaddress, const byte *start, const byte *end, bool write,
                                         bool execute, bool accessed, bool dirty) -> const byte * {
        const byte *v = reinterpret_cast<const byte *>(vaddress);
        while (start < end) {
            if (map)
                kmmap(start, v, table, PageLevel::KB_VPN, write, execute, accessed, dirty);
            else
                kmunmap(v, table);

            v += PAGE_FRAME_SIZE;
            start += PAGE_FRAME_SIZE;

            // kprintln("On loop:");
            // kspit(v);
            // kspit(start);
        }

        return v;
    };

    // Mapping text section
    // Non writable but executable
    const byte *text_begin = reinterpret_cast<const byte *>(&_text_start);
    const byte *text_end = reinterpret_cast<const byte *>(&_text_end);
    map_unmap_lambda(vaddress, text_begin, text_end, false, true, true, false);

    kspit(text_begin);
    kspit(text_end);
    kspit(vaddress);

    // Mapping read only data section
    // Non writable nor executable
    const byte *rodata_begin = reinterpret_cast<const byte *>(&_rodata_start);
    const byte *rodata_end = reinterpret_cast<const byte *>(&_rodata_end);
    vaddress = apply_offset(vaddress, rodata_begin - text_begin);
    map_unmap_lambda(vaddress, rodata_begin, rodata_end, false, false, true, false);

    kspit(rodata_begin);
    kspit(rodata_end);
    kspit(vaddress);

    // Mapping data section
    // Only readable/writable
    const byte *data_begin = reinterpret_cast<const byte *>(&_data_start);
    const byte *data_end = reinterpret_cast<const byte *>(&_data_end);
    vaddress = apply_offset(vaddress, data_begin - rodata_begin);
    map_unmap_lambda(vaddress, data_begin, data_end, true, false, true, true);

    kspit(data_begin);
    kspit(data_end);
    kspit(vaddress);

    // Mapping bss section
    // Only readable/writable
    const byte *bss_begin = reinterpret_cast<const byte *>(&_bss_start);
    const byte *bss_end = reinterpret_cast<const byte *>(&_bss_end);
    vaddress = apply_offset(vaddress, bss_begin - data_begin);
    map_unmap_lambda(vaddress, bss_begin, bss_end, true, false, true, true);

    kspit(bss_begin);
    kspit(bss_end);
    kspit(vaddress);

    // Mapping stack section
    // Only readable/writable
    const byte *stack_begin = reinterpret_cast<const byte *>(&_stack_start);
    const byte *stack_end = reinterpret_cast<const byte *>(&_stack_end);
    vaddress = apply_offset(vaddress, stack_begin - bss_begin);
    map_unmap_lambda(vaddress, stack_begin, stack_end, true, false, true, true);

    kspit(stack_begin);
    kspit(stack_end);
    kspit(vaddress);
}

void *kernel_start_physical = nullptr;

void identity_map_kernel(PageTable *table) {
    kernel_start_physical = reinterpret_cast<void *>(&_text_start);
    kspit(kernel_start_physical);
    map_kernel_impl(table, kernel_start_physical, true);
}
void identity_unmap_kernel(PageTable *table) {
    map_kernel_impl(table, kernel_start_physical, false);
}

void end_map_kernel(PageTable *table) {
    size_t kernel_size = reinterpret_cast<byte *>(&_stack_end) - reinterpret_cast<byte *>(&_text_start);
    void *vaddress = to_ptr(uintptr_t(0) - uintptr_t(0x40000000));
    map_kernel_impl(table, vaddress, true);
}

bool is_address_mapped(PageTable *table, void *vaddress) {
    return !get_physical_address(table, vaddress).is_error();
}

void setup_kernel_memory_mapping() {
    PageFrameManager &manager = PageFrameManager::instance();
    kernel_page_table = reinterpret_cast<PageTable *>(manager.get_frame().get_value());

    memset(kernel_page_table, 0, sizeof(PageTable));
    identity_map_kernel(kernel_page_table);
    print_table(kernel_page_table);
    end_map_kernel(kernel_page_table);
}

} // namespace hls