/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV-util implementation.

    DTV-util is free software: you can redistribute it and/or modify it 
  under the terms of the GNU Lesser General Public License as published by the Free 
  Software Foundation, either version 2 of the License.

    DTV-util is distributed in the hope that it will be useful, but WITHOUT ANY 
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV-util.

    DTV-util es Software Libre: Ud. puede redistribuirlo y/o modificarlo 
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV-util se distribuye esperando que resulte de utilidad, pero SIN NINGUNA 
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN 
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública 
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU 
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#pragma once

#include <algorithm>
#include <typeinfo>
#include <boost/utility/enable_if.hpp>
#include <boost/throw_exception.hpp>

#define ANY_CONDITION(T)  sizeof(T) <= sizeof(void *)

namespace util {

	class bad_any_cast : std::bad_cast {
	public:
		bad_any_cast(const std::type_info& src, const std::type_info& dest)
			: from(src.name()), to(dest.name())
		{ }
		virtual const char* what() {
			return "ginga::bad_any_cast: failed conversion using base::any_cast";
		}
		const char* from;
		const char* to;
	};

	namespace any { namespace detail {

		//	Create
		template <typename T>
		typename boost::enable_if_c<ANY_CONDITION(T)>::type
			create( void **obj, const T &x ) {
				new(obj) T(x);
		}

		template <typename T>
		typename boost::disable_if_c<ANY_CONDITION(T)>::type
			create( void **obj, const T &x ) {
				*obj = new T(x);
		}

		//	Get
		template <typename ResultType>
		typename boost::enable_if_c<ANY_CONDITION(ResultType), const ResultType &>::type
		get( void* const* obj ) {
			return *reinterpret_cast<ResultType const*>(obj);
		}

		template <typename ResultType>
		typename boost::disable_if_c<ANY_CONDITION(ResultType), const ResultType &>::type
		get( void* const* obj ) {
			return *reinterpret_cast<ResultType const*>(*obj);
		}

		//	Type
		template<typename T>
		const std::type_info &type( void ) {
			return typeid(T);
		}

		//	Delete
		template <typename T>
		typename boost::enable_if_c<ANY_CONDITION(T)>::type
			del( void **x ) {
			reinterpret_cast<T *>(x)->~T();
		}

		template <typename T>
		typename boost::disable_if_c<ANY_CONDITION(T)>::type
			del( void **x ) {
			delete (*reinterpret_cast<T**>(x));
		}

		//	Clone
		template <typename T>
		typename boost::enable_if_c<ANY_CONDITION(T)>::type
			clone( void* const* src, void **dest ) {
			new(dest) T(*reinterpret_cast<T const*>(src));
		}

		template <typename T>
		typename boost::disable_if_c<ANY_CONDITION(T)>::type
			clone( void* const* src, void **dest ) {
			*dest = new T(**reinterpret_cast<T* const*>(src));
		}

		//	Move
		template <typename T>
		typename boost::enable_if_c<ANY_CONDITION(T)>::type
			move( void* const* src, void **dest ) {
			reinterpret_cast<T*>(dest)->~T();
			*reinterpret_cast<T*>(dest) = *reinterpret_cast<T const*>(src);
		}

		template <typename T>
		typename boost::disable_if_c<ANY_CONDITION(T)>::type
			move( void* const* src, void **dest ) {
			(*reinterpret_cast<T**>(dest))->~T();
			**reinterpret_cast<T**>(dest) = **reinterpret_cast<T* const*>(src);
		}

		//	function pointer table
		struct fxn_ptr_table {
			const std::type_info& (*type)();
			void (*static_delete)( void ** );
			void (*clone)( void* const*, void ** );
			void (*move)( void* const*, void ** );
		};

		//	getTable
		template<typename T>
		struct getTable {
			static fxn_ptr_table* get() {
				static fxn_ptr_table static_table = {
					&type<T>,	//	type
					&del<T>,	//	delete
					&clone<T>,	//	clone
					&move<T>,	//	move
				};
				return &static_table;
			}
		};

	}}

	class Any {
	public:
		// structors
		template <typename T>
		Any( const T& x ) {
			_table = any::detail::getTable<T>::get();
			any::detail::create<T>( &_object, x );
		}

		Any( void ) {
			_table  = NULL;
			_object = NULL;
		}

		Any( const Any &x ) {
			_table  = NULL;
			_object = NULL;
			assign(x);
		}

		~Any() {
			reset();
		}

		//	queries
		bool empty() const {
			return _table == NULL;
		}

		const std::type_info &type() const {
			return empty() ? typeid(void *) : _table->type();
		}

		template<typename ValueType>
		bool operator==( const ValueType &val ) const {
			return _table == any::detail::getTable<ValueType>::get() && get<ValueType>() == val;
		}

		template<typename ResultType>
		const ResultType &get() const {
			if (empty() || type() != typeid(ResultType)) {
				throw bad_any_cast( type(), typeid(ResultType) );
			}
			return any::detail::get<ResultType>(&_object);
		}

		//	operations
		Any &operator=( const Any &other ) {
			assign( other );
			return *this;
		}

		template<typename ValueType>
		Any &operator=( const ValueType &val ) {
			assign(val);
			return *this;
		}

	protected:

		//	Assignment
		template <typename T>
		Any &assign( const T &x ) {
			//	are we copying between the same type?
			any::detail::fxn_ptr_table *x_table = any::detail::getTable<T>::get();
			if (_table != x_table) {
				reset();
				//	update table pointer
				_table = x_table;
			}
			else {
				_table->static_delete(&_object);
			}
			any::detail::create<T>( &_object, x );
			return *this;
		}

		Any &assign( const Any &x ) {
			//	are we copying between the same type?
			if (!empty() && !x.empty() && _table == x._table) {
				//	if so, we can avoid reallocation
				_table->move(&x._object, &_object);
			}
			else if (!x.empty()) {
				reset();
				x._table->clone(&x._object, &_object);
				_table = x._table;
			}
			else {
				reset();
			}
			return *this;
		}

		void reset() {
			if (!empty()) {
				_table->static_delete(&_object);
				_table  = NULL;
				_object = NULL;
			}
		}

	private:
		// fields
		any::detail::fxn_ptr_table *_table;
		void *_object;
	};

}
