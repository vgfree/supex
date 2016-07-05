/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2015, ruki All rights reserved.
 *
 * @author      ruki
 * @file        prefix.c
 *
 */
/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "../../tbox.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
#ifdef __tb_debug__
static tb_void_t tb_pool_data_dump_data(tb_byte_t const* data, tb_size_t size)
{
    // check
    tb_assert_and_check_return(data && size);

    // dump head
    tb_trace_i("");

    // walk
    tb_size_t           i = 0;
    tb_size_t           n = 147;
    tb_byte_t const*    p = data;
    tb_byte_t const*    e = data + size;
    tb_char_t           info[8192];
    while (p < e)
    {
        // full line?
        tb_char_t* q = info;
        tb_char_t* d = info + sizeof(info);
        if (p + 0x20 <= e)
        {
            // dump offset
            if (q < d) q += tb_snprintf(q, d - q, "%08X ", p - data);

            // dump data
            for (i = 0; i < 0x20; i++)
            {
                if (!(i & 3) && q < d) q += tb_snprintf(q, d - q, " ");
                if (q < d) q += tb_snprintf(q, d - q, " %02X", p[i]);
            }

            // dump spaces
            if (q < d) q += tb_snprintf(q, d - q, "  ");

            // dump characters
            for (i = 0; i < 0x20; i++)
            {
                if (q < d) q += tb_snprintf(q, d - q, "%c", tb_isgraph(p[i])? p[i] : '.');
            }

            // dump it
            if (q < d)
            {
                // end
                *q = '\0';

                // trace
                tb_trace_i("%s", info);
            }

            // update p
            p += 0x20;
        }
        // has left?
        else if (p < e)
        {
            // init padding
            tb_size_t padding = n - 0x20;

            // dump offset
            if (q < d) q += tb_snprintf(q, d - q, "%08X ", p - data); 
            if (padding >= 9) padding -= 9;

            // dump data
            tb_size_t left = e - p;
            for (i = 0; i < left; i++)
            {
                if (!(i & 3)) 
                {
                    if (q < d) q += tb_snprintf(q, d - q, " ");
                    if (padding) padding--;
                }

                if (q < d) q += tb_snprintf(q, d - q, " %02X", p[i]);
                if (padding >= 3) padding -= 3;
            }

            // dump spaces
            while (padding--) if (q < d) q += tb_snprintf(q, d - q, " ");
                
            // dump characters
            for (i = 0; i < left; i++)
            {
                if (q < d) q += tb_snprintf(q, d - q, "%c", tb_isgraph(p[i])? p[i] : '.');
            }

            // dump it
            if (q < d)
            {
                // end
                *q = '\0';

                // trace
                tb_trace_i("%s", info);
            }

            // update p
            p += left;
        }
        // end
        else break;
    }
}
__tb_no_sanitize_address__ tb_size_t tb_pool_data_size(tb_cpointer_t data)
{
    // check
    tb_check_return_val(data, 0);

    // done
    tb_size_t               size = 0;
    tb_pool_data_head_t*    data_head = tb_null;
    do
    {
        // tbox must be running normally
        tb_check_break(tb_state() == TB_STATE_OK);

        // get global allocator
        tb_allocator_ref_t allocator = tb_allocator();
        tb_check_break(allocator);

        // have this data address?
        tb_check_break(tb_allocator_have(allocator, data));

        // the data head
        data_head = &(((tb_pool_data_head_t*)data)[-1]);
        tb_check_break(data_head->debug.magic == TB_POOL_DATA_MAGIC);

        // ok
        size = data_head->size;

    } while (0);

    // ok?
    return size;
}
tb_void_t tb_pool_data_dump(tb_cpointer_t data, tb_bool_t verbose, tb_char_t const* prefix)
{
    // done
    tb_pool_data_head_t* data_head = tb_null;
    do
    {
        // no data?
        tb_assert_and_check_break(data);

        // the data head
        data_head = &(((tb_pool_data_head_t*)data)[-1]);
        
        // dump the head info
        tb_size_t data_limit = 256;
        if (data_head->debug.magic == TB_POOL_DATA_MAGIC)
        {
            // the data size
            tb_size_t data_size = (tb_size_t)data_head->size;

            // format the backtrace prefix
            tb_char_t backtrace_prefix[256] = {0};
            tb_snprintf(backtrace_prefix, sizeof(backtrace_prefix), "%s    ", prefix? prefix : "");

            // dump backtrace
            tb_size_t nframe = 0;
            while (nframe < tb_arrayn(data_head->debug.backtrace) && data_head->debug.backtrace[nframe]) nframe++;
            tb_trace_i("%sdata: from: %s(): %u, %s", prefix? prefix : "", data_head->debug.func, data_head->debug.line, data_head->debug.file);
            tb_backtrace_dump(backtrace_prefix, data_head->debug.backtrace, nframe);

            // dump the data info
            tb_trace_i("%sdata: %p, size: %lu, patch: %x", prefix? prefix : "", data, data_size, ((tb_byte_t const*)data)[data_size]);

            // dump the first 256-bytes data 
            if (data_size && verbose) 
            {
                // the dump size
                tb_size_t dump_size = tb_min(data_size, data_limit);

                // dump it
                tb_trace_i("%sdata: first %lu-bytes:", prefix? prefix : "", dump_size);
                tb_pool_data_dump_data((tb_byte_t const*)data, dump_size);

                // dump the last 256-bytes data 
                if (data_size > dump_size)
                {
                    // the last data
                    tb_byte_t const* data_last = tb_max((tb_byte_t const*)data + data_size - data_limit, (tb_byte_t const*)data + dump_size);
                    
                    // update the dump size
                    dump_size = (tb_byte_t const*)data + data_size - data_last;

                    // dump it
                    tb_trace_i("%sdata: last %lu-bytes:", prefix? prefix : "", dump_size);
                    tb_pool_data_dump_data(data_last, dump_size);
                }
            }
        }
        // for the public fixed_pool
        else if (data_head->debug.magic == TB_POOL_DATA_EMPTY_MAGIC)
        {
            // format the backtrace prefix
            tb_char_t backtrace_prefix[256] = {0};
            tb_snprintf(backtrace_prefix, sizeof(backtrace_prefix), "%s    ", prefix? prefix : "");

            // dump backtrace
            tb_size_t nframe = 0;
            while (nframe < tb_arrayn(data_head->debug.backtrace) && data_head->debug.backtrace[nframe]) nframe++;
            tb_trace_i("%sdata: from: %s(): %u, %s", prefix? prefix : "", data_head->debug.func, data_head->debug.line, data_head->debug.file);
            tb_backtrace_dump(backtrace_prefix, data_head->debug.backtrace, nframe);

            // dump the data info
            tb_trace_i("%sdata: %p, size: fixed", prefix? prefix : "", data);
        }
        else
        {
            // dump the data head
            tb_trace_i("%sdata: invalid head:", prefix? prefix : "");
            tb_pool_data_dump_data((tb_byte_t const*)data_head, sizeof(tb_pool_data_head_t));

            // dump the first 256-bytes data 
            tb_trace_i("%sdata: first %lu-bytes:", prefix? prefix : "", data_limit);
            tb_pool_data_dump_data((tb_byte_t const*)data, data_limit);
        }

    } while (0);
}
tb_void_t tb_pool_data_save_backtrace(tb_pool_data_debug_head_t* debug_head, tb_size_t skip_frames)
{ 
    tb_size_t nframe = tb_backtrace_frames(debug_head->backtrace, tb_arrayn(debug_head->backtrace), skip_frames + 2); 
    if (nframe < tb_arrayn(debug_head->backtrace)) tb_memset_(debug_head->backtrace + nframe, 0, (tb_arrayn(debug_head->backtrace) - nframe) * sizeof(tb_cpointer_t)); 
}
#endif

