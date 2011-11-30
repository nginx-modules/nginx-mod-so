/*
 * Copyright (C) Tsukasa Hamano <hamano@osstech.co.jp>
 */

#include <dlfcn.h>

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char *ngx_so(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t  ngx_so_commands[] = {
    { ngx_string("module"),
      NGX_MAIN_CONF|NGX_CONF_TAKE2,
      ngx_so,
      0,
      0,
      NULL },
      ngx_null_command
};

static ngx_core_module_t  ngx_so_module_ctx = {
    ngx_string("so"),
    NULL,
    NULL
};

ngx_module_t  ngx_so_module = {
    NGX_MODULE_V1,
    &ngx_so_module_ctx,            /* module context */
    ngx_so_commands,               /* module directives */
    NGX_CORE_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

static int so_num = 0;
static int so_max = 16; // see auto/modules and objs/ngx_modules.c

static char *
ngx_so(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    void *handle;
    ngx_module_t *sym;
    ngx_str_t *value = cf->args->elts;
    char *symname = (char *)value[1].data;
    char *filename = (char *)value[2].data;

    handle = dlopen(filename, RTLD_LAZY);
    if(!handle){
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "dlopen error: %s", dlerror());
        return NGX_CONF_ERROR;
    }
    sym = dlsym(handle, symname);
    if(!sym){
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "dlsym error: %s", dlerror());
        return NGX_CONF_ERROR;
    }

    if(so_num >= so_max){
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "so_max limit reached.");
        return NGX_CONF_ERROR;
    }

    sym->index = ngx_max_module;
    ngx_modules[ngx_max_module++] = sym;
    so_num++;

    return NGX_CONF_OK;
}