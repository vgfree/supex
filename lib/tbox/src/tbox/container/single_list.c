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
 * @file        single_list.c
 * @ingroup     container
 *
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME                "list"
#define TB_TRACE_MODULE_DEBUG               (0)

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "list.h"
#include "single_list_entry.h"
#include "../libc/libc.h"
#include "../math/math.h"
#include "../memory/memory.h"
#include "../stream/stream.h"
#include "../platform/platform.h"
#include "../algorithm/algorithm.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * macros
 */

// the list grow
#ifdef __tb_small__ 
#   define TB_SINGLE_LIST_GROW             (128)
#else
#   define TB_SINGLE_LIST_GROW             (256)
#endif

// the list maxn
#ifdef __tb_small__
#   define TB_SINGLE_LIST_MAXN             (1 << 16)
#else
#   define TB_SINGLE_LIST_MAXN             (1 << 30)
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

// the list impl type
typedef struct __tb_single_list_impl_t
{
    // the itor
    tb_iterator_t                   itor;

    // the pool
    tb_fixed_pool_ref_t             pool;

    // the head
    tb_single_list_entry_head_t     head;

    // the element
    tb_element_t                    element;

}tb_single_list_impl_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * private implementation
 */
static tb_size_t tb_single_list_itor_size(tb_iterator_ref_t iterator)
{
    // the size
    return tb_single_list_size((tb_single_list_ref_t)iterator);
}
static tb_size_t tb_single_list_itor_head(tb_iterator_ref_t iterator)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)iterator;
    tb_assert(impl);

    // head
    return (tb_size_t)tb_single_list_entry_head(&impl->head);
}
static tb_size_t tb_single_list_itor_last(tb_iterator_ref_t iterator)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)iterator;
    tb_assert(impl);

    // last
    return (tb_size_t)tb_single_list_entry_last(&impl->head);
}
static tb_size_t tb_single_list_itor_tail(tb_iterator_ref_t iterator)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)iterator;
    tb_assert(impl);

    // tail
    return (tb_size_t)tb_single_list_entry_tail(&impl->head);
}
static tb_size_t tb_single_list_itor_next(tb_iterator_ref_t iterator, tb_size_t itor)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)iterator;
    tb_assert(impl && itor);

    // next
    return (tb_size_t)tb_single_list_entry_next(&impl->head, (tb_single_list_entry_t*)itor);
}
static tb_pointer_t tb_single_list_itor_item(tb_iterator_ref_t iterator, tb_size_t itor)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)iterator;
    tb_assert(impl && itor);

    // data
    return impl->element.data(&impl->element, (tb_cpointer_t)(((tb_single_list_entry_t*)itor) + 1));
}
static tb_void_t tb_single_list_itor_copy(tb_iterator_ref_t iterator, tb_size_t itor, tb_cpointer_t item)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)iterator;
    tb_assert(impl && itor);

    // copy
    impl->element.copy(&impl->element, (tb_pointer_t)(((tb_single_list_entry_t*)itor) + 1), item);
}
static tb_long_t tb_single_list_itor_comp(tb_iterator_ref_t iterator, tb_cpointer_t litem, tb_cpointer_t ritem)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)iterator;
    tb_assert(impl && impl->element.comp);

    // comp
    return impl->element.comp(&impl->element, litem, ritem);
}
static tb_void_t tb_single_list_itor_remove_range(tb_iterator_ref_t iterator, tb_size_t prev, tb_size_t next, tb_size_t size)
{
    // no size?
    tb_check_return(size);

    // the list size
    tb_size_t single_list_size = tb_single_list_size((tb_single_list_ref_t)iterator);
    tb_check_return(single_list_size);

    // limit size
    if (size > single_list_size) size = single_list_size;

    // remove the body items
    if (prev) 
    {
        // done
        tb_bool_t end = tb_false;
        while (size-- && !end) 
        {
            // end?
            end = (tb_iterator_next((tb_single_list_ref_t)iterator, prev) == next)? tb_true : tb_false;

            // remove next
            tb_single_list_remove_next((tb_single_list_ref_t)iterator, prev);
        }
    }
    // remove the head items
    else 
    {
        while (size--) tb_single_list_remove_head((tb_single_list_ref_t)iterator);
    }
}
static tb_void_t tb_single_list_item_exit(tb_pointer_t data, tb_cpointer_t priv)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)priv;
    tb_assert_and_check_return(impl);

    // free data
    if (impl->element.free) impl->element.free(&impl->element, (tb_pointer_t)(((tb_single_list_entry_t*)data) + 1));
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_single_list_ref_t tb_single_list_init(tb_size_t grow, tb_element_t element)
{
    // check
    tb_assert_and_check_return_val(element.size && element.data && element.dupl && element.repl, tb_null);

    // done
    tb_bool_t               ok = tb_false;
    tb_single_list_impl_t*  impl = tb_null;
    do
    {
        // using the default grow
        if (!grow) grow = TB_SINGLE_LIST_GROW;

        // make list
        impl = tb_malloc0_type(tb_single_list_impl_t);
        tb_assert_and_check_break(impl);

        // init element
        impl->element = element;

        // init iterator
        impl->itor.mode         = TB_ITERATOR_MODE_FORWARD;
        impl->itor.priv         = tb_null;
        impl->itor.step         = element.size;
        impl->itor.size         = tb_single_list_itor_size;
        impl->itor.head         = tb_single_list_itor_head;
        impl->itor.last         = tb_single_list_itor_last;
        impl->itor.tail         = tb_single_list_itor_tail;
        impl->itor.next         = tb_single_list_itor_next;
        impl->itor.item         = tb_single_list_itor_item;
        impl->itor.copy         = tb_single_list_itor_copy;
        impl->itor.comp         = tb_single_list_itor_comp;
        impl->itor.remove_range = tb_single_list_itor_remove_range;

        // init pool, item = entry + data
        impl->pool = tb_fixed_pool_init(tb_null, grow, sizeof(tb_single_list_entry_t) + element.size, tb_null, tb_single_list_item_exit, (tb_cpointer_t)impl);
        tb_assert_and_check_break(impl->pool);

        // init head
        tb_single_list_entry_init_(&impl->head, 0, sizeof(tb_single_list_entry_t) + element.size, tb_null);

        // ok
        ok = tb_true;

    } while (0);

    // failed?
    if (!ok)
    {
        // exit it
        if (impl) tb_single_list_exit((tb_single_list_ref_t)impl);
        impl = tb_null;
    }

    // ok?
    return (tb_single_list_ref_t)impl;
}
tb_void_t tb_single_list_exit(tb_single_list_ref_t list)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return(impl);
   
    // clear data
    tb_single_list_clear((tb_single_list_ref_t)impl);

    // free pool
    if (impl->pool) tb_fixed_pool_exit(impl->pool);

    // free it
    tb_free(impl);
}
tb_void_t tb_single_list_clear(tb_single_list_ref_t list)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return(impl);

    // clear pool
    if (impl->pool) tb_fixed_pool_clear(impl->pool);

    // clear head
    tb_single_list_entry_clear(&impl->head);
}
tb_pointer_t tb_single_list_head(tb_single_list_ref_t list)
{
    return tb_iterator_item(list, tb_iterator_head(list));
}
tb_pointer_t tb_single_list_last(tb_single_list_ref_t list)
{
    return tb_iterator_item(list, tb_iterator_last(list));
}
tb_size_t tb_single_list_size(tb_single_list_ref_t list)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return_val(impl && impl->pool, 0);
    tb_assert(tb_single_list_entry_size(&impl->head) == tb_fixed_pool_size(impl->pool));

    // the size
    return tb_single_list_entry_size(&impl->head);
}
tb_size_t tb_single_list_maxn(tb_single_list_ref_t list)
{
    // the item maxn
    return TB_SINGLE_LIST_MAXN;
}
tb_size_t tb_single_list_insert_next(tb_single_list_ref_t list, tb_size_t itor, tb_cpointer_t data)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return_val(impl && impl->element.dupl && impl->pool, 0);

    // full?
    tb_assert_and_check_return_val(tb_single_list_size(list) < tb_single_list_maxn(list), tb_iterator_tail(list));

    // the node
    tb_single_list_entry_ref_t node = (tb_single_list_entry_ref_t)itor;
    tb_assert_and_check_return_val(node, tb_iterator_tail(list));

    // make entry
    tb_single_list_entry_ref_t entry = (tb_single_list_entry_ref_t)tb_fixed_pool_malloc(impl->pool);
    tb_assert_and_check_return_val(entry, tb_iterator_tail(list));

    // init entry data
    impl->element.dupl(&impl->element, (tb_pointer_t)(((tb_single_list_entry_t*)entry) + 1), data);

    // insert it
    tb_single_list_entry_insert_next(&impl->head, node, entry);

    // ok
    return (tb_size_t)entry;
}
tb_size_t tb_single_list_insert_head(tb_single_list_ref_t list, tb_cpointer_t data)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return_val(impl && impl->element.dupl && impl->pool, 0);

    // full?
    tb_assert_and_check_return_val(tb_single_list_size(list) < tb_single_list_maxn(list), tb_iterator_tail(list));

    // make entry
    tb_single_list_entry_ref_t entry = (tb_single_list_entry_ref_t)tb_fixed_pool_malloc(impl->pool);
    tb_assert_and_check_return_val(entry, tb_iterator_tail(list));

    // init entry data
    impl->element.dupl(&impl->element, (tb_pointer_t)(((tb_single_list_entry_t*)entry) + 1), data);

    // insert it
    tb_single_list_entry_insert_head(&impl->head, entry);

    // ok
    return (tb_size_t)entry;
}
tb_size_t tb_single_list_insert_tail(tb_single_list_ref_t list, tb_cpointer_t data)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return_val(impl && impl->element.dupl && impl->pool, 0);

    // full?
    tb_assert_and_check_return_val(tb_single_list_size(list) < tb_single_list_maxn(list), tb_iterator_tail(list));

    // make entry
    tb_single_list_entry_ref_t entry = (tb_single_list_entry_ref_t)tb_fixed_pool_malloc(impl->pool);
    tb_assert_and_check_return_val(entry, tb_iterator_tail(list));

    // init entry data
    impl->element.dupl(&impl->element, (tb_pointer_t)(((tb_single_list_entry_t*)entry) + 1), data);

    // insert it
    tb_single_list_entry_insert_tail(&impl->head, entry);

    // ok
    return (tb_size_t)entry;
}
tb_void_t tb_single_list_replace(tb_single_list_ref_t list, tb_size_t itor, tb_cpointer_t data)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return(impl && impl->element.repl && itor);

    // the node
    tb_single_list_entry_ref_t node = (tb_single_list_entry_ref_t)itor;
    tb_assert_and_check_return(node);

    // replace data
    impl->element.repl(&impl->element, (tb_pointer_t)(((tb_single_list_entry_t*)node) + 1), data);
}
tb_void_t tb_single_list_replace_head(tb_single_list_ref_t list, tb_cpointer_t data)
{
    tb_single_list_replace(list, tb_iterator_head(list), data);
}
tb_void_t tb_single_list_replace_last(tb_single_list_ref_t list, tb_cpointer_t data)
{
    tb_single_list_replace(list, tb_iterator_last(list), data);
}
tb_void_t tb_single_list_remove_next(tb_single_list_ref_t list, tb_size_t itor)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return(impl && impl->pool && itor);

    // the node
    tb_single_list_entry_ref_t node = (tb_single_list_entry_ref_t)itor;
    tb_assert_and_check_return(node);

    // the next node
    tb_single_list_entry_ref_t next = tb_single_list_entry_next(&impl->head, node);
    tb_assert_and_check_return(next);

    // remove next node
    tb_single_list_entry_remove_next(&impl->head, node);

    // free next node
    tb_fixed_pool_free(impl->pool, next);
}
tb_void_t tb_single_list_remove_head(tb_single_list_ref_t list)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return(impl && impl->pool);

    // the node
    tb_single_list_entry_ref_t node = tb_single_list_entry_head(&impl->head);
    tb_assert_and_check_return(node);

    // remove next node
    tb_single_list_entry_remove_head(&impl->head);

    // free head node
    tb_fixed_pool_free(impl->pool, node);
}
#ifdef __tb_debug__
tb_void_t tb_single_list_dump(tb_single_list_ref_t list)
{
    // check
    tb_single_list_impl_t* impl = (tb_single_list_impl_t*)list;
    tb_assert_and_check_return(impl);

    // trace
    tb_trace_i("single_list: size: %lu", tb_single_list_size(list));

    // done
    tb_char_t cstr[4096];
    tb_for_all (tb_pointer_t, data, list)
    {
        // trace
        if (impl->element.cstr) 
        {
            tb_trace_i("    %s", impl->element.cstr(&impl->element, data, cstr, sizeof(cstr)));
        }
        else
        {
            tb_trace_i("    %p", data);
        }
    }
}
#endif
