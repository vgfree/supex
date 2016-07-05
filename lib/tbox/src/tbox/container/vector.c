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
 * @file        vector.c
 * @ingroup     container
 *
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME                "vector"
#define TB_TRACE_MODULE_DEBUG               (0)

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "vector.h"
#include "../libc/libc.h"
#include "../utils/utils.h"
#include "../memory/memory.h"
#include "../stream/stream.h"
#include "../platform/platform.h"
#include "../algorithm/algorithm.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * macros
 */

// the vector grow
#ifdef __tb_small__ 
#   define TB_VECTOR_GROW             (128)
#else
#   define TB_VECTOR_GROW             (256)
#endif

// the vector maxn
#ifdef __tb_small__
#   define TB_VECTOR_MAXN             (1 << 16)
#else
#   define TB_VECTOR_MAXN             (1 << 30)
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

// the vector impl type
typedef struct __tb_vector_impl_t
{
    // the itor
    tb_iterator_t           itor;

    // the data
    tb_byte_t*              data;

    // the size
    tb_size_t               size;

    // the grow
    tb_size_t               grow;

    // the maxn
    tb_size_t               maxn;

    // the element
    tb_element_t            element;

}tb_vector_impl_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * private implementation
 */
static tb_size_t tb_vector_itor_size(tb_iterator_ref_t iterator)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl);

    // size
    return impl->size;
}
static tb_size_t tb_vector_itor_head(tb_iterator_ref_t iterator)
{
    // head
    return 0;
}
static tb_size_t tb_vector_itor_last(tb_iterator_ref_t iterator)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl);

    // last
    return impl->size? impl->size - 1 : 0;
}
static tb_size_t tb_vector_itor_tail(tb_iterator_ref_t iterator)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl);

    // tail
    return impl->size;
}
static tb_size_t tb_vector_itor_next(tb_iterator_ref_t iterator, tb_size_t itor)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl);
    tb_assert_and_check_return_val(itor < impl->size, impl->size);

    // next
    return itor + 1;
}
static tb_size_t tb_vector_itor_prev(tb_iterator_ref_t iterator, tb_size_t itor)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl);
    tb_assert_and_check_return_val(itor && itor <= impl->size, 0);

    // prev
    return itor - 1;
}
static tb_pointer_t tb_vector_itor_item(tb_iterator_ref_t iterator, tb_size_t itor)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert_and_check_return_val(impl && itor < impl->size, tb_null);
    
    // data
    return impl->element.data(&impl->element, impl->data + itor * iterator->step);
}
static tb_void_t tb_vector_itor_copy(tb_iterator_ref_t iterator, tb_size_t itor, tb_cpointer_t item)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl);

    // copy
    impl->element.copy(&impl->element, impl->data + itor * iterator->step, item);
}
static tb_long_t tb_vector_itor_comp(tb_iterator_ref_t iterator, tb_cpointer_t litem, tb_cpointer_t ritem)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl && impl->element.comp);

    // comp
    return impl->element.comp(&impl->element, litem, ritem);
}
static tb_void_t tb_vector_itor_remove(tb_iterator_ref_t iterator, tb_size_t itor)
{
    // remove it
    tb_vector_remove((tb_vector_ref_t)iterator, itor);
}
static tb_void_t tb_vector_itor_remove_range(tb_iterator_ref_t iterator, tb_size_t prev, tb_size_t next, tb_size_t size)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)iterator;
    tb_assert(impl);

    // remove the items
    if (size) tb_vector_nremove((tb_vector_ref_t)iterator, prev != impl->size? prev + 1 : 0, size);
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_vector_ref_t tb_vector_init(tb_size_t grow, tb_element_t element)
{
    // check
    tb_assert_and_check_return_val(element.size && element.data && element.dupl && element.repl && element.ndupl && element.nrepl, tb_null);

    // done
    tb_bool_t           ok = tb_false;
    tb_vector_impl_t*   impl = tb_null;
    do
    {
        // using the default grow
        if (!grow) grow = TB_VECTOR_GROW;

        // make impl
        impl = tb_malloc0_type(tb_vector_impl_t);
        tb_assert_and_check_break(impl);

        // init impl
        impl->size      = 0;
        impl->grow      = grow;
        impl->maxn      = grow;
        impl->element   = element;
        tb_assert_and_check_break(impl->maxn < TB_VECTOR_MAXN);

        // init iterator
        impl->itor.mode         = TB_ITERATOR_MODE_FORWARD | TB_ITERATOR_MODE_REVERSE | TB_ITERATOR_MODE_RACCESS | TB_ITERATOR_MODE_MUTABLE;
        impl->itor.priv         = tb_null;
        impl->itor.step         = element.size;
        impl->itor.size         = tb_vector_itor_size;
        impl->itor.head         = tb_vector_itor_head;
        impl->itor.last         = tb_vector_itor_last;
        impl->itor.tail         = tb_vector_itor_tail;
        impl->itor.prev         = tb_vector_itor_prev;
        impl->itor.next         = tb_vector_itor_next;
        impl->itor.item         = tb_vector_itor_item;
        impl->itor.copy         = tb_vector_itor_copy;
        impl->itor.comp         = tb_vector_itor_comp;
        impl->itor.remove       = tb_vector_itor_remove;
        impl->itor.remove_range = tb_vector_itor_remove_range;

        // make data
        impl->data = (tb_byte_t*)tb_nalloc0(impl->maxn, element.size);
        tb_assert_and_check_break(impl->data);

        // ok
        ok = tb_true;

    } while (0);

    // failed?
    if (!ok)
    {
        // exit it
        if (impl) tb_vector_exit((tb_vector_ref_t)impl);
        impl = tb_null;
    }

    // ok?
    return (tb_vector_ref_t)impl;
}
tb_void_t tb_vector_exit(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl);

    // clear data
    tb_vector_clear(vector);

    // free data
    if (impl->data) tb_free(impl->data);
    impl->data = tb_null;

    // free it
    tb_free(impl);
}
tb_void_t tb_vector_clear(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl);

    // free data
    if (impl->element.nfree)
        impl->element.nfree(&impl->element, impl->data, impl->size);

    // reset size 
    impl->size = 0;
}
tb_void_t tb_vector_copy(tb_vector_ref_t vector, tb_vector_ref_t hcopy)
{
    // check
    tb_vector_impl_t*       impl = (tb_vector_impl_t*)vector;
    tb_vector_impl_t const* copy = (tb_vector_impl_t const*)hcopy;
    tb_assert_and_check_return(impl && copy);

    // check element
    tb_assert_and_check_return(impl->element.type == copy->element.type);
    tb_assert_and_check_return(impl->element.size == copy->element.size);

    // check itor
    tb_assert_and_check_return(impl->itor.mode == copy->itor.mode);
    tb_assert_and_check_return(impl->itor.step == copy->itor.step);

    // null? clear it
    if (!copy->size) 
    {
        tb_vector_clear(vector);
        return ;
    }
    
    // resize if small
    if (impl->size < copy->size) tb_vector_resize(vector, copy->size);
    tb_assert_and_check_return(impl->data && copy->data && impl->size >= copy->size);

    // copy data
    if (copy->data != impl->data) tb_memcpy(impl->data, copy->data, copy->size * copy->element.size);

    // copy size
    impl->size = copy->size;
}
tb_pointer_t tb_vector_data(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return_val(impl, tb_null);

    // data
    return impl->data;
}
tb_pointer_t tb_vector_head(tb_vector_ref_t vector)
{
    return tb_iterator_item(vector, tb_iterator_head(vector));
}
tb_pointer_t tb_vector_last(tb_vector_ref_t vector)
{
    return tb_iterator_item(vector, tb_iterator_last(vector));
}
tb_size_t tb_vector_size(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t const* impl = (tb_vector_impl_t const*)vector;
    tb_assert_and_check_return_val(impl, 0);

    // size
    return impl->size;
}
tb_size_t tb_vector_grow(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t const* impl = (tb_vector_impl_t const*)vector;
    tb_assert_and_check_return_val(impl, 0);

    // grow
    return impl->grow;
}
tb_size_t tb_vector_maxn(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t const* impl = (tb_vector_impl_t const*)vector;
    tb_assert_and_check_return_val(impl, 0);

    // maxn
    return impl->maxn;
}
tb_bool_t tb_vector_resize(tb_vector_ref_t vector, tb_size_t size)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return_val(impl, tb_false);
    
    // free items if the impl is decreased
    if (size < impl->size)
    {
        // free data
        if (impl->element.nfree) 
            impl->element.nfree(&impl->element, impl->data + size * impl->element.size, impl->size - size);
    }

    // resize buffer
    if (size > impl->maxn)
    {
        tb_size_t maxn = tb_align4(size + impl->grow);
        tb_assert_and_check_return_val(maxn < TB_VECTOR_MAXN, tb_false);

        // realloc data
        impl->data = (tb_byte_t*)tb_ralloc(impl->data, maxn * impl->element.size);
        tb_assert_and_check_return_val(impl->data, tb_false);

        // must be align by 4-bytes
        tb_assert_and_check_return_val(!(((tb_size_t)(impl->data)) & 3), tb_false);

        // clear the grow data
        tb_memset(impl->data + impl->size * impl->element.size, 0, (maxn - impl->maxn) * impl->element.size);

        // save maxn
        impl->maxn = maxn;
    }

    // update size
    impl->size = size;
    return tb_true;
}
tb_void_t tb_vector_insert_prev(tb_vector_ref_t vector, tb_size_t itor, tb_cpointer_t data)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && impl->data && impl->element.size && itor <= impl->size);

    // save size
    tb_size_t osize = impl->size;

    // grow a item
    if (!tb_vector_resize(vector, osize + 1)) 
    {
        tb_trace_d("impl resize: %u => %u failed", osize, osize + 1);
        return ;
    }

    // move items if not at tail
    if (osize != itor) tb_memmov(impl->data + (itor + 1) * impl->element.size, impl->data + itor * impl->element.size, (osize - itor) * impl->element.size);

    // save data
    impl->element.dupl(&impl->element, impl->data + itor * impl->element.size, data);
}
tb_void_t tb_vector_insert_next(tb_vector_ref_t vector, tb_size_t itor, tb_cpointer_t data)
{
    tb_vector_insert_prev(vector, tb_iterator_next(vector, itor), data);
}
tb_void_t tb_vector_insert_head(tb_vector_ref_t vector, tb_cpointer_t data)
{
    tb_vector_insert_prev(vector, 0, data);
}
tb_void_t tb_vector_insert_tail(tb_vector_ref_t vector, tb_cpointer_t data)
{
    tb_vector_insert_prev(vector, tb_vector_size(vector), data);
}
tb_void_t tb_vector_ninsert_prev(tb_vector_ref_t vector, tb_size_t itor, tb_cpointer_t data, tb_size_t size)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && impl->data && size && itor <= impl->size);

    // save size
    tb_size_t osize = impl->size;

    // grow size
    if (!tb_vector_resize(vector, osize + size)) 
    {
        tb_trace_d("impl resize: %u => %u failed", osize, osize + 1);
        return ;
    }

    // move items if not at tail
    if (osize != itor) tb_memmov(impl->data + (itor + size) * impl->element.size, impl->data + itor * impl->element.size, (osize - itor) * impl->element.size);

    // duplicate data
    impl->element.ndupl(&impl->element, impl->data + itor * impl->element.size, data, size);
}
tb_void_t tb_vector_ninsert_next(tb_vector_ref_t vector, tb_size_t itor, tb_cpointer_t data, tb_size_t size)
{
    tb_vector_ninsert_prev(vector, tb_iterator_next(vector, itor), data, size);
}
tb_void_t tb_vector_ninsert_head(tb_vector_ref_t vector, tb_cpointer_t data, tb_size_t size)
{
    tb_vector_ninsert_prev(vector, 0, data, size);
}
tb_void_t tb_vector_ninsert_tail(tb_vector_ref_t vector, tb_cpointer_t data, tb_size_t size)
{
    tb_vector_ninsert_prev(vector, tb_vector_size(vector), data, size);
}
tb_void_t tb_vector_replace(tb_vector_ref_t vector, tb_size_t itor, tb_cpointer_t data)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && impl->data && itor <= impl->size);

    // replace data
    impl->element.repl(&impl->element, impl->data + itor * impl->element.size, data);
}
tb_void_t tb_vector_replace_head(tb_vector_ref_t vector, tb_cpointer_t data)
{
    tb_vector_replace(vector, 0, data);
}
tb_void_t tb_vector_replace_last(tb_vector_ref_t vector, tb_cpointer_t data)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && impl->size);

    // replace
    tb_vector_replace(vector, impl->size - 1, data);
}
tb_void_t tb_vector_nreplace(tb_vector_ref_t vector, tb_size_t itor, tb_cpointer_t data, tb_size_t size)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && impl->data && impl->size && itor <= impl->size && size);

    // strip size
    if (itor + size > impl->size) size = impl->size - itor;

    // replace data
    impl->element.nrepl(&impl->element, impl->data + itor * impl->element.size, data, size);
}
tb_void_t tb_vector_nreplace_head(tb_vector_ref_t vector, tb_cpointer_t data, tb_size_t size)
{
    tb_vector_nreplace(vector, 0, data, size);
}
tb_void_t tb_vector_nreplace_last(tb_vector_ref_t vector, tb_cpointer_t data, tb_size_t size)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && impl->size && size);

    // replace
    tb_vector_nreplace(vector, size >= impl->size? 0 : impl->size - size, data, size);
}
tb_void_t tb_vector_remove(tb_vector_ref_t vector, tb_size_t itor)
{   
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && itor < impl->size);

    if (impl->size)
    {
        // do free
        if (impl->element.free) impl->element.free(&impl->element, impl->data + itor * impl->element.size);

        // move data if itor is not last
        if (itor < impl->size - 1) tb_memmov(impl->data + itor * impl->element.size, impl->data + (itor + 1) * impl->element.size, (impl->size - itor - 1) * impl->element.size);

        // resize
        impl->size--;
    }
}
tb_void_t tb_vector_remove_head(tb_vector_ref_t vector)
{
    tb_vector_remove(vector, 0);
}
tb_void_t tb_vector_remove_last(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl);

    if (impl->size)
    {
        // do free
        if (impl->element.free) impl->element.free(&impl->element, impl->data + (impl->size - 1) * impl->element.size);

        // resize
        impl->size--;
    }
}
tb_void_t tb_vector_nremove(tb_vector_ref_t vector, tb_size_t itor, tb_size_t size)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && size && itor < impl->size);

    // clear it
    if (!itor && size >= impl->size) 
    {
        tb_vector_clear(vector);
        return ;
    }
    
    // strip size
    if (itor + size > impl->size) size = impl->size - itor;

    // compute the left size
    tb_size_t left = impl->size - itor - size;

    // free data
    if (impl->element.nfree)
        impl->element.nfree(&impl->element, impl->data + itor * impl->element.size, size);

    // move the left data
    if (left)
    {
        tb_byte_t* pd = impl->data + itor * impl->element.size;
        tb_byte_t* ps = impl->data + (itor + size) * impl->element.size;
        tb_memmov(pd, ps, left * impl->element.size);
    }

    // update size
    impl->size -= size;
}
tb_void_t tb_vector_nremove_head(tb_vector_ref_t vector, tb_size_t size)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && size);

    // clear it
    if (size >= impl->size)
    {
        tb_vector_clear(vector);
        return ;
    }

    // remove head
    tb_vector_nremove(vector, 0, size);
}
tb_void_t tb_vector_nremove_last(tb_vector_ref_t vector, tb_size_t size)
{   
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl && size);

    // clear it
    if (size >= impl->size)
    {
        tb_vector_clear(vector);
        return ;
    }

    // remove last
    tb_vector_nremove(vector, impl->size - size, size);
}
#ifdef __tb_debug__
tb_void_t tb_vector_dump(tb_vector_ref_t vector)
{
    // check
    tb_vector_impl_t* impl = (tb_vector_impl_t*)vector;
    tb_assert_and_check_return(impl);

    // trace
    tb_trace_i("vector: size: %lu", tb_vector_size(vector));

    // done
    tb_char_t cstr[4096];
    tb_for_all (tb_pointer_t, data, vector)
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
