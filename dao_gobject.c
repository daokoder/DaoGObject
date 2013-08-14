/*
// Dao Standard Modules
// http://www.daovm.net
//
// Copyright (c) 2013, Limin Fu
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED  BY THE COPYRIGHT HOLDERS AND  CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED  WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO,  THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL  THE COPYRIGHT HOLDER OR CONTRIBUTORS  BE LIABLE FOR ANY DIRECT,
// INDIRECT,  INCIDENTAL, SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL  DAMAGES (INCLUDING,
// BUT NOT LIMITED TO,  PROCUREMENT OF  SUBSTITUTE  GOODS OR  SERVICES;  LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY OF
// LIABILITY,  WHETHER IN CONTRACT,  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include"string.h"
#include"daoType.h"
#include"daoValue.h"
#include"daoClass.h"
#include"daoObject.h"
#include"daoRoutine.h"
#include"daoNumtype.h"
#include"daoProcess.h"
#include"daoVmspace.h"
#include"daoNamespace.h"
#include"daoParser.h"
#include"daoPlatforms.h"
#include"daoGC.h"

#include"ffi.h"
#include"girepository.h"
#include"gibaseinfo.h"

const char *const ctype[] =
{ "uint8", "sint8", "uint16", "sint16", "uint32", "sint32", "uint64", "sint64" };

const char *const alias[] =
{ "uchar", "char", "ushort", "short", "uint", "int", "uint64", "sint64" };

typedef void* (*bare_func)();
typedef struct DaoxFFI DaoxFFI;

enum DaoxFFITypes
{
	DAOX_FFI_DAOINT ,
	DAOX_FFI_UINT8 ,
	DAOX_FFI_SINT8 ,
	DAOX_FFI_UINT16 ,
	DAOX_FFI_SINT16 ,
	DAOX_FFI_UINT32 ,
	DAOX_FFI_SINT32 ,
	DAOX_FFI_UINT64 ,
	DAOX_FFI_SINT64 ,
};
DaoType *daox_type_none  = NULL;
DaoType *daox_type_sint8 = NULL;
DaoType *daox_type_uint8 = NULL;
DaoType *daox_type_sint16 = NULL;
DaoType *daox_type_uint16 = NULL;
DaoType *daox_type_sint32 = NULL;
DaoType *daox_type_uint32 = NULL;
DaoType *daox_type_sint64 = NULL;
DaoType *daox_type_uint64 = NULL;
DaoType *daox_type_float  = NULL;
DaoType *daox_type_double = NULL;
DaoType *daox_type_string = NULL;
DaoType *daox_type_wstring = NULL;
DaoType *daox_type_stream = NULL;
DaoType *daox_type_cdata = NULL;

struct DaoxFFI
{
	DAO_CSTRUCT_COMMON;

	void      *fptr;
	ffi_cif    cif;
	ffi_type  *args[ DAO_MAX_PARAM + 1 ];
	ffi_type  *retype;
};
DaoType *daox_type_daoffi = NULL;

DaoxFFI* DaoxFFI_New()
{
	DaoxFFI *self = (DaoxFFI*) dao_calloc( 1, sizeof(DaoxFFI) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_daoffi );
	return self;
}
void DaoxFFI_Delete( DaoxFFI *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	dao_free( self );
}


typedef struct DaoxGITypeInfo DaoxGITypeInfo;

struct DaoxGITypeInfo
{
	DAO_CSTRUCT_COMMON;

	DaoTypeBase  *typer;
	GIBaseInfo   *baseinfo;
};
DaoType *daox_type_gitypeinfo = NULL;

DaoxGITypeInfo* DaoxGITypeInfo_New( GIBaseInfo *baseinfo )
{
	DaoxGITypeInfo *self = (DaoxGITypeInfo*) dao_calloc( 1, sizeof(DaoxGITypeInfo) );
	DaoCstruct_Init( (DaoCstruct*) self, daox_type_gitypeinfo );
	self->typer = dao_calloc( 1, sizeof(DaoTypeBase) );
	self->typer->name = g_base_info_get_name( baseinfo );
	self->baseinfo = g_base_info_ref( baseinfo );
	return self;
}
void DaoxGITypeInfo_Delete( DaoxGITypeInfo *self )
{
	DaoCstruct_Free( (DaoCstruct*) self );
	g_base_info_unref( self->baseinfo );
	self->typer->name = NULL;
	dao_free( self->typer );
	dao_free( self );
}


typedef union DaoxFFIArgument DaoxFFIArgument;

union DaoxFFIArgument
{
	daoint    m_daoint;
	gint8     m_sint8;
	guint8    m_uint8;
	gint16    m_sint16;
	guint16   m_uint16;
	gint32    m_sint32;
	guint32   m_uint32;
	gint64    m_sint64;
	guint64   m_uint64;
	gfloat    m_float;
	gdouble   m_double;
	gpointer  m_pointer;
	ffi_arg   m_ffiarg;
	char*     m_mbstring;
	wchar_t*  m_wcstring;
};

static void* DaoValue_MakeFFIData( DaoValue *self, DaoType *type, DaoxFFIArgument *buffer )
{
	DaoType *itp;
	DaoArray *array;
	daoint ivalue;
	switch( type->tid ){
	case DAO_INTEGER :
		ivalue = self->xInteger.value;
		switch( type->ffitype ){
		case DAOX_FFI_SINT8  : buffer->m_sint8  = ivalue; break;
		case DAOX_FFI_UINT8  : buffer->m_uint8  = ivalue; break;
		case DAOX_FFI_SINT16 : buffer->m_sint16 = ivalue; break;
		case DAOX_FFI_UINT16 : buffer->m_uint16 = ivalue; break;
		case DAOX_FFI_SINT32 : buffer->m_sint32 = ivalue; break;
		case DAOX_FFI_UINT32 : buffer->m_uint32 = ivalue; break;
		case DAOX_FFI_SINT64 : buffer->m_sint64 = ivalue; break;
		case DAOX_FFI_UINT64 : buffer->m_uint64 = ivalue; break;
		default : buffer->m_daoint = ivalue; break;
		}
		return buffer;
	case DAO_FLOAT :
		return & self->xFloat.value;
	case DAO_DOUBLE :
		return & self->xDouble.value;
	case DAO_COMPLEX :
		return & self->xComplex.value;
	case DAO_STRING :
		if( type == daox_type_string && self->xString.data->mbs == NULL ){
			DString_ToMBS( self->xString.data );
		}else if( type == daox_type_wstring && self->xString.data->wcs == NULL ){
			DString_ToWCS( self->xString.data );
		}
		if( self->xString.data->mbs ) return & self->xString.data->mbs;
		return & self->xString.data->wcs;
	case DAO_ARRAY :
		itp = type->nested->size ? type->nested->items.pType[0] : NULL;
		array = (DaoArray*) self;
		if( itp == NULL ) return & array->data.p;
		switch( itp->tid ){
		case DAO_INTEGER :
			switch( type->ffitype ){
			case DAOX_FFI_SINT8  : DaoArray_ToSByte( array ); break;
			case DAOX_FFI_UINT8  : DaoArray_ToUByte( array ); break;
			case DAOX_FFI_SINT16 : DaoArray_ToSShort( array ); break;
			case DAOX_FFI_UINT16 : DaoArray_ToUShort( array ); break;
			case DAOX_FFI_SINT32 : DaoArray_ToSInt( array ); break;
			case DAOX_FFI_UINT32 : DaoArray_ToUInt( array ); break;
			/*case DAOX_FFI_SINT64 : DaoArray_To( array ); break;*/
			/*case DAOX_FFI_UINT64 : DaoArray_To( array ); break;*/
			default : break;
			}
			break;
		case DAO_FLOAT :
			DaoArray_ToFloat( array );
			break;
		case DAO_DOUBLE :
			DaoArray_ToDouble( array );
			break;
		case DAO_COMPLEX :
			break;
		default : break;
		}
		return & array->data.p;
	case DAO_CDATA :
		return & self->xCdata.data;
	default : break;
	}
	return NULL;
}

static void DaoCLoader_Execute( DaoProcess *proc, DaoValue *p[], int N )
{
	DaoRoutine *func = proc->topFrame->routine;
	DaoType *routype, *tp, *itp, **nested;
	DaoArray *array;
	DaoCdata *cdata;
	DaoxFFI *ffi;
	DString *str = NULL;
	complex16 com = { 0.0, 0.0 };
	DaoxFFIArgument retvalue = {0};
	DaoxFFIArgument parvalues[DAO_MAX_PARAM];
	void *args[DAO_MAX_PARAM];
	daoint ivalue = 0;
	int i;

	ffi = (DaoxFFI*) DaoValue_CastCstruct( DaoList_Back( func->routConsts ), daox_type_daoffi );
	if( ffi == NULL ){
		return;
	}
	routype = func->routType;
	nested = routype->nested->items.pType;
	memset( args, 0, func->parCount * sizeof(void*) );
	for(i=0; i<func->parCount; i++){
		if( i >= routype->nested->size ) continue;
		tp = nested[i];
		if( tp->tid == DAO_PAR_NAMED || tp->tid == DAO_PAR_DEFAULT ) tp = (DaoType*) tp->aux;
		args[i] = DaoValue_MakeFFIData( p[i], tp, & parvalues[i] );
		if( args[i] == NULL ) args[i] = & p[i];
	}
	ffi_call( & ffi->cif, ffi->fptr, & retvalue, args );
	tp = (DaoType*) routype->aux;
	if( tp ){
		switch( tp->tid ){
		case DAO_INTEGER :
			switch( tp->ffitype ){
			case DAOX_FFI_SINT8  : ivalue = retvalue.m_sint8  ; break;
			case DAOX_FFI_UINT8  : ivalue = retvalue.m_uint8  ; break;
			case DAOX_FFI_SINT16 : ivalue = retvalue.m_sint16 ; break;
			case DAOX_FFI_UINT16 : ivalue = retvalue.m_uint16 ; break;
			case DAOX_FFI_SINT32 : ivalue = retvalue.m_sint32 ; break;
			case DAOX_FFI_UINT32 : ivalue = retvalue.m_uint32 ; break;
			case DAOX_FFI_SINT64 : ivalue = retvalue.m_sint64 ; break;
			case DAOX_FFI_UINT64 : ivalue = retvalue.m_uint64 ; break;
			default : ivalue = retvalue.m_daoint ; break;
			}
			DaoProcess_PutInteger( proc, ivalue );
			break;
		case DAO_FLOAT :
			DaoProcess_PutFloat( proc, retvalue.m_float );
			break;
		case DAO_DOUBLE :
			DaoProcess_PutDouble( proc, retvalue.m_double );
			break;
		//case DAO_COMPLEX :
		//	return & self->xComplex.value;
		case DAO_STRING :
			if( tp == daox_type_string ){
				DaoProcess_PutMBString( proc, retvalue.m_mbstring );
			}else{
				DaoProcess_PutWCString( proc, retvalue.m_wcstring );
			}
			break;
		case DAO_CDATA :
			DaoProcess_PutCdata( proc, retvalue.m_pointer, tp );
			break;
		default : break;
		}
	}
	for(i=0; i<func->parCount; i++){
		if( i >= routype->nested->size ) continue;
		tp = nested[i];
		if( tp->tid == DAO_PAR_NAMED || tp->tid == DAO_PAR_DEFAULT ) tp = (DaoType*) tp->aux;
		switch( tp->tid ){
		case DAO_STRING :
			if( p[i]->xString.data->mbs ){
				int len = strlen( p[i]->xString.data->mbs );
				if( len < p[i]->xString.data->size )
					p[i]->xString.data->size = strlen( p[i]->xString.data->mbs );
			}else{
				int len = wcslen( p[i]->xString.data->wcs );
				if( len < p[i]->xString.data->size )
					p[i]->xString.data->size = wcslen( p[i]->xString.data->wcs );
			}
			break;
		case DAO_ARRAY :
			itp = tp->nested->size ? tp->nested->items.pType[0] : NULL;
			array = (DaoArray*) p[i];
			if( itp == NULL ) break;
			switch( itp->tid ){
			case DAO_INTEGER :
				switch( itp->ffitype ){
				case DAOX_FFI_SINT8  : DaoArray_FromSByte( array ); break;
				case DAOX_FFI_UINT8  : DaoArray_FromUByte( array ); break;
				case DAOX_FFI_SINT16 : DaoArray_FromSShort( array ); break;
				case DAOX_FFI_UINT16 : DaoArray_FromUShort( array ); break;
				case DAOX_FFI_SINT32 : DaoArray_FromSInt( array ); break;
				case DAOX_FFI_UINT32 : DaoArray_FromUInt( array ); break;
				/*case DAOX_FFI_SINT64 : DaoArray_From( array ); break;*/
				/*case DAOX_FFI_UINT64 : DaoArray_From( array ); break;*/
				default : break;
				}
				break;
			case DAO_FLOAT :
				DaoArray_FromFloat( array );
				break;
			case DAO_DOUBLE :
				DaoArray_FromDouble( array );
				break;
			case DAO_COMPLEX :
				break;
			default : break;
			}
			break;
		default : break;
		}
	}
}


static void DaoGobject_QName( GIBaseInfo *info, DString *qname )
{
	DString_SetMBS( qname, g_base_info_get_namespace( info ) );
	DString_AppendMBS( qname, "::" );
	DString_AppendMBS( qname, g_base_info_get_name( info ) );
}

static int DaoGobject_ConvertType( GITypeInfo *gitype, ffi_type **ffitype, DaoType **daotype, DaoNamespace *ns )
{
	*ffitype = NULL;
	*daotype = NULL;
	//printf( "DaoGobject_ConvertType: tag = %i\n", g_type_info_get_tag( gitype ) );
	if( g_type_info_is_pointer( gitype ) ){
		*ffitype = & ffi_type_pointer;
		switch( g_type_info_get_tag( gitype ) ){
		case GI_TYPE_TAG_INTERFACE :
			{
				GIRepository *girepo = g_irepository_get_default();
				GIBaseInfo *baseinfo = g_type_info_get_interface( gitype );
				GIInfoType  itype = g_base_info_get_type( baseinfo );
				DString *qname = DString_New(1);

				DaoGobject_QName( baseinfo, qname );
				DaoType *type = DaoNamespace_FindType( ns, qname );
				*daotype = type;
				if( itype == GI_INFO_TYPE_OBJECT ){
				}
				g_base_info_unref( baseinfo );
				DString_Delete( qname );
			}
			break;
		case GI_TYPE_TAG_ARRAY :
			{
				GITypeInfo *elemtype = g_type_info_get_param_type( gitype, 0 );
				switch( g_type_info_get_tag( elemtype ) ){
				case GI_TYPE_TAG_INT8    : *daotype = daox_type_sint8; break;
				case GI_TYPE_TAG_UINT8   : *daotype = daox_type_uint8; break;
				case GI_TYPE_TAG_INT16   : *daotype = daox_type_sint16; break;
				case GI_TYPE_TAG_UINT16  : *daotype = daox_type_uint16; break;
				case GI_TYPE_TAG_INT32   : *daotype = daox_type_sint32; break;
				case GI_TYPE_TAG_UINT32  : *daotype = daox_type_uint32; break;
				case GI_TYPE_TAG_INT64   : *daotype = daox_type_sint64; break;
				case GI_TYPE_TAG_UINT64  : *daotype = daox_type_uint64; break;
				case GI_TYPE_TAG_FLOAT   : *daotype = daox_type_float;  break;
				case GI_TYPE_TAG_DOUBLE  : *daotype = daox_type_double; break;
				default : break;
				}
				*daotype = DaoNamespace_MakeType( ns, "array", DAO_ARRAY, NULL, daotype, 1 );
				break;
			}
		case GI_TYPE_TAG_VOID    : *daotype = daox_type_cdata; break;
		case GI_TYPE_TAG_INT8    : *daotype = daox_type_sint8; break;
		case GI_TYPE_TAG_UINT8   : *daotype = daox_type_uint8; break;
		case GI_TYPE_TAG_INT16   : *daotype = daox_type_sint16; break;
		case GI_TYPE_TAG_UINT16  : *daotype = daox_type_uint16; break;
		case GI_TYPE_TAG_INT32   : *daotype = daox_type_sint32; break;
		case GI_TYPE_TAG_UINT32  : *daotype = daox_type_uint32; break;
		case GI_TYPE_TAG_INT64   : *daotype = daox_type_sint64; break;
		case GI_TYPE_TAG_UINT64  : *daotype = daox_type_uint64; break;
		case GI_TYPE_TAG_FLOAT   : *daotype = daox_type_float;  break;
		case GI_TYPE_TAG_DOUBLE  : *daotype = daox_type_double; break;
		case GI_TYPE_TAG_UTF8    : *daotype = daox_type_string; break;
		case GI_TYPE_TAG_UNICHAR : *daotype = daox_type_wstring; break;
		default : break;
		}
	}else{
		switch( g_type_info_get_tag( gitype ) ){
		case GI_TYPE_TAG_VOID :
			*ffitype = & ffi_type_void;
			*daotype = daox_type_none;
			break;
		case GI_TYPE_TAG_INT8 :
			*ffitype = & ffi_type_sint8;
			*daotype = daox_type_sint8;
			break;
		case GI_TYPE_TAG_UINT8 :
			*ffitype = & ffi_type_uint8;
			*daotype = daox_type_uint8;
			break;
		case GI_TYPE_TAG_INT16 :
			*ffitype = & ffi_type_sint16;
			*daotype = daox_type_sint16;
			break;
		case GI_TYPE_TAG_UINT16 :
			*ffitype = & ffi_type_uint16;
			*daotype = daox_type_uint16;
			break;
		case GI_TYPE_TAG_INT32 :
			*ffitype = & ffi_type_sint32;
			*daotype = daox_type_sint32;
			break;
		case GI_TYPE_TAG_UINT32 :
			*ffitype = & ffi_type_uint32;
			*daotype = daox_type_uint32;
			break;
		case GI_TYPE_TAG_INT64 :
			*ffitype = & ffi_type_sint64;
			*daotype = daox_type_sint64;
			break;
		case GI_TYPE_TAG_UINT64 :
			*ffitype = & ffi_type_uint64;
			*daotype = daox_type_uint64;
			break;
		case GI_TYPE_TAG_FLOAT :
			*ffitype = & ffi_type_float;
			*daotype = daox_type_float;
			break;
		case GI_TYPE_TAG_DOUBLE :
			*ffitype = & ffi_type_double;
			*daotype = daox_type_double;
			break;
		default : break;
		}
	}
	return 1;
}

static int DaoGobject_WrapFunction( DaoNamespace *ns, GIFunctionInfo *info, DaoType *host )
{
	gpointer symbol;
	GITypelib *typelib = g_base_info_get_typelib( info );
	DaoxFFI *daoffi;
	GITypeInfo *ret_gitype;
	ffi_type *ffitype = NULL;
	DaoType *daotype = NULL;
	DArray *ffitypes = DArray_New(0);
	DArray *daotypes = DArray_New(0);
	DaoRoutine *routine = DaoRoutine_New( ns, NULL, 0 );
	int i, argc = g_callable_info_get_n_args( info );

	DString_SetMBS( routine->routName, g_base_info_get_name( info ) );
	if( host ){
		routine->attribs |= DAO_ROUT_PARSELF;
		daotype = DaoNamespace_MakeType( ns, "self", DAO_PAR_NAMED, (DaoValue*)host, NULL, 0 );
		DArray_Append( daotypes, daotype );
		DArray_Append( ffitypes, & ffi_type_pointer );
		DaoList_Append( routine->routConsts, NULL );
	}
	for(i=0; i<argc; ++i){
		GIArgInfo *arg = g_callable_info_get_arg( info, i );
		GITypeInfo *argtype = g_arg_info_get_type( arg );
		const char *argname = g_base_info_get_name( arg );
		DaoGobject_ConvertType( argtype, & ffitype, & daotype, ns );
		daotype = DaoNamespace_MakeType( ns, argname, DAO_PAR_NAMED, (DaoValue*)daotype, NULL, 0 );
		DArray_Append( daotypes, daotype );
		DArray_Append( ffitypes, ffitype );
		DaoList_Append( routine->routConsts, NULL );
	}
	ret_gitype = g_callable_info_get_return_type( info );
	DaoGobject_ConvertType( ret_gitype, & ffitype, & daotype, ns );

	routine->pFunc = DaoCLoader_Execute;
	routine->parCount = daotypes->size;
	routine->routType = DaoNamespace_MakeType( ns, "routine", DAO_ROUTINE, (DaoValue*)daotype, daotypes->items.pType, daotypes->size );
	GC_IncRC( routine->routType );

	g_typelib_symbol( typelib, g_function_info_get_symbol( info ), & symbol );

	daoffi = DaoxFFI_New();
	daoffi->fptr = symbol;
	daoffi->retype = ffitype;
	for(i=0; i<ffitypes->size; ++i) daoffi->args[i] = (ffi_type*) ffitypes->items.pVoid[i];
	DArray_Delete( ffitypes );
	DArray_Delete( daotypes );

	if( ffi_prep_cif( & daoffi->cif, FFI_DEFAULT_ABI, routine->parCount, daoffi->retype, daoffi->args ) != FFI_OK ){
		DaoxFFI_Delete( daoffi );
		DaoGC_TryDelete( (DaoValue*) routine );
		return 0;
	}
	DaoList_Append( routine->routConsts, (DaoValue*) daoffi );
	if( host ){
		routine->routType->attrib |= DAO_TYPE_SELF;
		DaoMethods_Insert( host->kernel->methods, routine, ns, host );
	}else{
		DaoNamespace_AddConst( ns, routine->routName, (DaoValue*) routine, DAO_DATA_PUBLIC );
	}
	return 1;
}

static int DaoGobject_SetupValues( DaoNamespace *self, DaoTypeBase *typer )
{
	//printf( "DaoGobject_SetupValues\n" );
	typer->core->kernel->SetupValues = NULL;
	return 1;
}
static int DaoGobject_SetupMethods( DaoNamespace *ns, DaoTypeBase *typer )
{
	DString name = DString_WrapMBS( "__gi_type_info__" );
	DNode *it = DMap_Find( typer->core->kernel->values, & name );
	DaoValue *value = it ? it->value.pValue : NULL;
	DaoxGITypeInfo *gitype = (DaoxGITypeInfo*) DaoValue_CastCstruct( value, daox_type_gitypeinfo );
	int i, num;
	typer->core->kernel->SetupMethods = NULL;
	if( gitype == NULL ) return 0;
	num = g_object_info_get_n_methods( gitype->baseinfo );
	//printf( "DaoGobject_SetupMethods: %p %i\n", gitype, num );
	if( num == 0 ) return 1;
	/* TODO: locking; */
	typer->core->kernel->methods = DHash_New( D_STRING, D_VALUE );
	for(i=0; i<num; ++i){
		GIFunctionInfo *finfo = g_object_info_get_method( gitype->baseinfo, i );
		DaoGobject_WrapFunction( ns, finfo, gitype->typer->core->kernel->abtype );
	}
	return 1;
}

static int DaoGobject_WrapType( DaoProcess *proc, DaoNamespace *ns, GIObjectInfo *info )
{
	DString *qname = DString_New(1);
	DaoxGITypeInfo* gitype = DaoxGITypeInfo_New( info );
	DaoType *daotype = DaoNamespace_WrapType( ns, gitype->typer, 1 );

	DaoGobject_QName( info, qname );
	DaoNamespace_AddType( ns, qname, daotype );

	DString_SetMBS( qname, "__gi_type_info__" );
	daotype->kernel->values = DHash_New( D_STRING, D_VALUE );
	DMap_Insert( daotype->kernel->values, qname, gitype );

	daotype->kernel->SetupValues = DaoGobject_SetupValues;
	daotype->kernel->SetupMethods = DaoGobject_SetupMethods;
	DString_Delete( qname );
	return 1;
}

static void DaoGobject_Load( DaoProcess *proc, DaoValue *p[], int N )
{
	char *nspace = DaoValue_TryGetMBString( p[0] );
	char *version = DaoValue_TryGetMBString( p[1] );
	char *dir = DaoValue_TryGetMBString( p[2] );
	GIRepository *girepo = g_irepository_get_default();
	GITypelib *typelib = NULL;
	GError *error = NULL;
	DaoVmSpace *vms = proc->vmSpace;
	DaoNamespace *ns;
	int i, ninfo;
	if( N < 3 ){
		typelib = g_irepository_require( girepo, nspace, version, 0, & error );
	}else{
		typelib = g_irepository_require_private( girepo, dir, nspace, version, 0, & error );
	}
	if( error ){
		printf( "%p %s\n", typelib, error->message );
		return;
	}
	ns = DaoNamespace_New( vms, nspace );
	DaoProcess_PutValue( proc, (DaoValue*) ns );
	ninfo = g_irepository_get_n_infos( girepo, nspace );
	for(i=0; i<ninfo; ++i){
		GIBaseInfo *baseinfo = g_irepository_get_info( girepo, nspace, i );
		GIInfoType gitype = g_base_info_get_type( baseinfo );
		const char *name = g_base_info_get_name( baseinfo );
		if( gitype == GI_INFO_TYPE_FUNCTION ) DaoGobject_WrapFunction( ns, baseinfo, NULL );
		if( gitype == GI_INFO_TYPE_OBJECT ) DaoGobject_WrapType( proc, ns, baseinfo );
	}
}


DaoTypeBase DaoxFFI_Typer =
{ "FFI", NULL, NULL, NULL, {0}, {0}, (FuncPtrDel) DaoxFFI_Delete, NULL };


DaoTypeBase DaoxGITypeInfo_Typer =
{ "GITypeInfo", NULL, NULL, NULL, {0}, {0}, (FuncPtrDel) DaoxGITypeInfo_Delete, NULL };


int DaoOnLoad( DaoVmSpace *vms, DaoNamespace *ns )
{
	DaoNamespace *io = DaoVmSpace_GetNamespace( vms, "io" );
	int i;

	daox_type_none = DaoNamespace_FindTypeMBS( ns, "none" );
	daox_type_float = DaoNamespace_FindTypeMBS( ns, "float" );
	daox_type_double = DaoNamespace_FindTypeMBS( ns, "double" );
	daox_type_stream = DaoNamespace_FindTypeMBS( io, "stream" );
	daox_type_cdata = DaoNamespace_FindTypeMBS( io, "cdata" );
	daox_type_string = DaoNamespace_FindTypeMBS( ns, "string" );
	daox_type_wstring = DaoNamespace_TypeDefine( ns, "string", "wstring" );
	daox_type_sint8  = DaoNamespace_TypeDefine( ns, "int", "sint8" );
	daox_type_uint8  = DaoNamespace_TypeDefine( ns, "int", "uint8" );
	daox_type_sint16 = DaoNamespace_TypeDefine( ns, "int", "sint16" );
	daox_type_uint16 = DaoNamespace_TypeDefine( ns, "int", "uint16" );
	daox_type_sint32 = DaoNamespace_TypeDefine( ns, "int", "sint32" );
	daox_type_uint32 = DaoNamespace_TypeDefine( ns, "int", "uint32" );
	daox_type_sint64 = DaoNamespace_TypeDefine( ns, "int", "sint64" );
	daox_type_uint64 = DaoNamespace_TypeDefine( ns, "int", "uint64" );
	daox_type_sint8->ffitype  = DAOX_FFI_SINT8;
	daox_type_uint8->ffitype  = DAOX_FFI_UINT8;
	daox_type_sint16->ffitype = DAOX_FFI_SINT16;
	daox_type_uint16->ffitype = DAOX_FFI_UINT16;
	daox_type_sint32->ffitype = DAOX_FFI_SINT32;
	daox_type_uint32->ffitype = DAOX_FFI_UINT32;
	daox_type_sint64->ffitype = DAOX_FFI_SINT64;
	daox_type_uint64->ffitype = DAOX_FFI_UINT64;

	daox_type_daoffi = DaoNamespace_WrapType( ns, & DaoxFFI_Typer, 0 );
	daox_type_gitypeinfo = DaoNamespace_WrapType( ns, & DaoxGITypeInfo_Typer, 0 );

	DaoNamespace_WrapFunction( ns, DaoGobject_Load,
			"gir_load( nspace : string, version = '', dir = '' ) => any" );
	return 0;
}
