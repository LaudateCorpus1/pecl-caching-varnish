/*-
 * Copyright (c) 2011 Anatoliy Belsky
 * All rights reserved.
 *
 * Author: Anatoliy Belsky <ab@php.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_varnish.h"

#include "varnish_lib.h"

/*ZEND_DECLARE_MODULE_GLOBALS(varnish)*/

/* True global resources - no need for thread safety here */
static int le_varnish;

/* Class entry definition */
zend_class_entry *VarnishAdmin_ce;
zend_class_entry *VarnishStat_ce;
zend_class_entry *VarnishException_ce;

/* {{{ varnish_functions[]
 */
const zend_function_entry varnish_functions[] = {
	{NULL, NULL, NULL}	
};
/* }}} */

/* {{{ VarnishAdmin_methods[]
 */
const zend_function_entry VarnishAdmin_methods[] = {
	PHP_ME(VarnishAdmin, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, connect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, auth, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setParam, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, stop, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, banUrl, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, isRunning, NULL, ZEND_ACC_PUBLIC)
#ifdef PHP_VARNISH_DEBUG
	PHP_ME(VarnishAdmin, test, NULL, ZEND_ACC_PUBLIC)
#endif
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ VarnishStat_methods{} */
const zend_function_entry VarnishStat_methods[] = {
	PHP_ME(VarnishStat, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishStat, getSnapshot, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ varnish_module_entry
 */
zend_module_entry varnish_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"varnish",
	varnish_functions,
	PHP_MINIT(varnish),
	PHP_MSHUTDOWN(varnish),
	NULL,
	NULL,
	PHP_MINFO(varnish),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_VARNISH_EXT_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_VARNISH
ZEND_GET_MODULE(varnish)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("varnish.global_value",	  "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_varnish_globals, varnish_globals)
	STD_PHP_INI_ENTRY("varnish.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_varnish_globals, varnish_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_varnish_globals_ctor
 */
/*static void php_varnish_globals_ctor(zend_varnish_globals *varnish_globals)
{
}*/
/* }}} */

/* {{{ php_varnish_globals_dtor
 */
/*static void php_varnish_globals_dtor(zend_varnish_globals *varnish_globals)
{

}*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(varnish)
{
	zend_class_entry ce;

	/* REGISTER_INI_ENTRIES();*/
	/*ZEND_INIT_MODULE_GLOBALS(varnish, php_varnish_globals_ctor, php_varnish_globals_dtor);*/

	/* Init internal classes */
	INIT_CLASS_ENTRY(ce, "VarnishAdmin", VarnishAdmin_methods);
	ce.create_object = php_varnish_adm_obj_init;
	VarnishAdmin_ce = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "VarnishStat", VarnishStat_methods);
	ce.create_object = php_varnish_stat_obj_init;
	VarnishStat_ce = zend_register_internal_class(&ce TSRMLS_CC);

	/* Init exceptions */
	INIT_CLASS_ENTRY(ce, "VarnishException", NULL);
	VarnishException_ce = zend_register_internal_class_ex(
		&ce, NULL, "exception" TSRMLS_CC
	);  

	REGISTER_LONG_CONSTANT("VARNISH_STATUS_SYNTAX", PHP_VARNISH_STATUS_SYNTAX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_UNKNOWN", PHP_VARNISH_STATUS_UNKNOWN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_UNIMPL", PHP_VARNISH_STATUS_UNIMPL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_TOOFEW", PHP_VARNISH_STATUS_TOOFEW, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_TOOMANY", PHP_VARNISH_STATUS_TOOMANY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_PARAM", PHP_VARNISH_STATUS_PARAM, CONST_CS | CONST_PERSISTENT);  
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_AUTH", PHP_VARNISH_STATUS_AUTH, CONST_CS | CONST_PERSISTENT);   
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_OK", PHP_VARNISH_STATUS_OK, CONST_CS | CONST_PERSISTENT);	 
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_CANT", PHP_VARNISH_STATUS_CANT, CONST_CS | CONST_PERSISTENT);   
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_COMMS", PHP_VARNISH_STATUS_COMMS, CONST_CS | CONST_PERSISTENT);  
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_CLOSE", PHP_VARNISH_STATUS_CLOSE, CONST_CS | CONST_PERSISTENT);  

	return SUCCESS;
}
/*  }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(varnish)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(varnish)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "varnish support", "enabled");
	php_info_print_table_row(2, "Version", PHP_VARNISH_EXT_VERSION);
	php_info_print_table_end();

	/* 
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */