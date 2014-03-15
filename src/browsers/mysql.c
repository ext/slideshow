/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2012 David Sveningsson <ext@sidvind.com>
 *
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "browser.h"
#include "core/log.h"
#include <string.h>
#include <mysql/mysql.h>

typedef struct {
	struct browser_module_t module;

	int loop_queue;
	unsigned int queue_id;
	int prev_slide_id;

	MYSQL* conn;

	MYSQL_STMT* stmt_slide;
	MYSQL_STMT* stmt_looping;
	MYSQL_STMT* stmt_pop;
} my;

MODULE_INFO("MySQL Browser", BROWSER_MODULE, "David Sveningsson");

static int pop_intermediate(my* this, int id);

static MYSQL_STMT* prepare(MYSQL* conn, const char* query){
	MYSQL_STMT* stmt = mysql_stmt_init(conn);
	if ( !stmt ){
		log_message(Log_Fatal, "mysql_stmt_init failed (out of memory?)\n");
		return NULL;
	}

	if ( mysql_stmt_prepare(stmt, query, strlen(query)) != 0 ){
		log_message(Log_Fatal, "mysql_stmt_prepare: %s\n", mysql_stmt_error(stmt));
		mysql_stmt_close(stmt);
		return NULL;
	}

	return stmt;
}

static int connect(my* this){
	this->conn = mysql_init(NULL);

	const browser_context_t* ctx = &this->module.context;
	if (!mysql_real_connect(this->conn, ctx->host, ctx->user, ctx->pass, ctx->name, 0, NULL, 0)) {
		log_message(Log_Fatal, "mysql_real_connect could not connect to database: %s\n", mysql_error(this->conn));
		return 1;
	}

	this->stmt_slide =
		prepare(this->conn,
		        "	SELECT" /* select from intermediate queue */
		        "		id, "
		        "		path, "
		        "		sortorder, "
		        "		queue_id, "
		        "		assembler "
		        "	FROM"
		        "		slide "
		        "	WHERE"
		        "		queue_id = -1 " /* -1 is intermediate queue */
		        "UNION "
		        "	SELECT" /* select next slide from regular queue */
		        "		id, "
		        "		path, "
		        "		sortorder, "
		        "		queue_id, "
		        "		assembler "
		        "	FROM"
		        "		slide "
		        "	WHERE"
		        "		queue_id = ? AND "
		        "		sortorder > ? "
		        "ORDER BY"
		        "	queue_id, "
		        "	sortorder "
		        "LIMIT 1");

	this->stmt_looping = prepare(this->conn, "SELECT `loop` FROM `queue` WHERE `id` = ? LIMIT 1");
	this->stmt_pop = prepare(this->conn, "UPDATE `slide` SET `queue_id` = 0 WHERE `id` = ?");
	if ( !(this->stmt_slide && this->stmt_looping && this->stmt_pop) ){
		return 1;
	}

	return 0;
}

static slide_context_t next_slide(my* this){
	slide_context_t slide;
	slide.filename = NULL;
	slide.assembler = NULL;

	MYSQL_BIND param[2];
	memset(param, 0, sizeof(param));
	param[0].buffer_type    = MYSQL_TYPE_LONG;
	param[0].buffer         = (void *)&this->queue_id;
	param[0].is_unsigned    = 1;
	param[0].is_null        = 0;
	param[0].length         = 0;
	param[1].buffer_type    = MYSQL_TYPE_LONG;
	param[1].buffer         = (void *)&this->prev_slide_id;
	param[1].is_unsigned    = 0;
	param[1].is_null        = 0;
	param[1].length         = 0;

	if ( mysql_stmt_bind_param(this->stmt_slide, param) != 0 ){
		log_message(Log_Warning, "mysql_stmt_bind_param failed: %s\n", mysql_stmt_error(this->stmt_slide));
		return slide;
	}

	if ( mysql_stmt_execute(this->stmt_slide) != 0 ){
		log_message(Log_Warning, "mysql_stmt_execute failed: %s\n", mysql_stmt_error(this->stmt_slide));
		return slide;
	}

	int id;
	char path[4096] = {0,};
	int sort_order;
	int queue_id;
	char assembler[128] = {0,};

	MYSQL_BIND result[5];
	memset(result, 0, sizeof(result));
	result[0].buffer_type    = MYSQL_TYPE_LONG;
	result[0].buffer         = (void *)&id;
	result[1].buffer_type    = MYSQL_TYPE_STRING;
	result[1].buffer         = (void *)path;
	result[1].buffer_length  = sizeof(path);
	result[2].buffer_type    = MYSQL_TYPE_LONG;
	result[2].buffer         = (void *)&sort_order;
	result[3].buffer_type    = MYSQL_TYPE_LONG;
	result[3].buffer         = (void *)&queue_id;
	result[4].buffer_type    = MYSQL_TYPE_STRING;
	result[4].buffer         = (void *)assembler;
	result[4].buffer_length  = sizeof(assembler);

	if ( mysql_stmt_bind_result(this->stmt_slide, result) != 0 ){
		log_message(Log_Warning, "mysql_stmt_bind_result failed: %s\n", mysql_stmt_error(this->stmt_slide));
		return slide;
	}

	int ret;
	switch ( (ret=mysql_stmt_fetch(this->stmt_slide)) ){
	case 0: /* row */
		mysql_stmt_free_result(this->stmt_slide);

		slide.filename = strdup(path);
		slide.assembler = strdup(assembler);

		log_message(Log_Info, "slide: %s\n", slide.filename);
		log_message(Log_Debug, "\tid: %d\n", id);
		log_message(Log_Debug, "\tsort_order: %d\n", sort_order);
		log_message(Log_Debug, "\tqueue_id: %d\n", queue_id);
		log_message(Log_Debug, "\tassembler: %s\n", slide.assembler);

		/* only update id if it comes from a regular queue, i.e., not from intermediate queue. */
		if ( queue_id > 0 ){
			this->prev_slide_id = sort_order;
		} else {
			/* pop intermediate slides back to unsorted */
			log_message(Log_Debug, "popping intermediate slide\n");
			pop_intermediate(this, id);
		}

		return slide;

	case MYSQL_NO_DATA:
		mysql_stmt_free_result(this->stmt_slide);

		/* empty queue */
		if ( this->prev_slide_id == -1 ){
			log_message(Log_Debug, "queue empty and prev_id is -1\n");
			log_message(Log_Debug, "\tqueue_id: %d\n", this->queue_id);
			return slide;
		}

		if ( !this->loop_queue ){
			log_message(Log_Info, "Queue finished and looping is diabled\n");
			log_message(Log_Debug, "\tqueue_id: %d\n", this->queue_id);
			return slide;
		}

		log_message(Log_Debug, "queue wrapping\n");
		this->prev_slide_id = -1;

		return next_slide(this);

	case 1: /* error */
	default:
		log_message(Log_Warning, "mysql_stmt_fetch failed: %s\n", mysql_stmt_error(this->stmt_slide));
		log_message(Log_Debug, "\tqueue_id: %d\n", this->queue_id);
		log_message(Log_Debug, "\told_id: %d\n", this->prev_slide_id);
		mysql_stmt_free_result(this->stmt_slide);
		return slide;
	}
}

static int pop_intermediate(my* this, int id){
	MYSQL_BIND param[1];
	memset(param, 0, sizeof(param));
	param[0].buffer_type    = MYSQL_TYPE_LONG;
	param[0].buffer         = (void *)&id;
	param[0].is_unsigned    = 1;

	if ( mysql_stmt_bind_param(this->stmt_pop, param) != 0 ){
		log_message(Log_Warning, "mysql_stmt_bind_param failed: %s\n", mysql_stmt_error(this->stmt_pop));
		return 1;
	}

	if ( mysql_stmt_execute(this->stmt_pop) != 0 ){
		log_message(Log_Warning, "mysql_stmt_execute failed: %s\n", mysql_stmt_error(this->stmt_pop));
		return 1;
	}

	return 0;
}

static int queue_reload(my* this){
	return 0;
}

static int queue_set(my* this, unsigned int id){
	log_message(Log_Debug, "queue_set(%d)\n", id);

	/* if we change queue we reset the position back to the start */
	if ( this->queue_id != id ){
		this->prev_slide_id = -1;
	}

	this->queue_id = id;

	MYSQL_BIND param[1];
	memset(param, 0, sizeof(param));
	param[0].buffer_type    = MYSQL_TYPE_LONG;
	param[0].buffer         = (void *)&this->queue_id;
	param[0].is_unsigned    = 1;

	if ( mysql_stmt_bind_param(this->stmt_looping, param) != 0 ){
		log_message(Log_Warning, "mysql_stmt_bind_param failed: %s\n", mysql_stmt_error(this->stmt_looping));
		return 1;
	}

	if ( mysql_stmt_execute(this->stmt_looping) != 0 ){
		log_message(Log_Warning, "mysql_stmt_execute failed: %s\n", mysql_stmt_error(this->stmt_looping));
		return 1;
	}

	MYSQL_BIND result[1];
	memset(result, 0, sizeof(result));
	result[0].buffer_type    = MYSQL_TYPE_LONG;
	result[0].buffer         = (void *)&this->loop_queue;

	if ( mysql_stmt_bind_result(this->stmt_looping, result) != 0 ){
		log_message(Log_Warning, "mysql_stmt_bind_result failed: %s\n", mysql_stmt_error(this->stmt_looping));
		return 1;
	}

	if ( mysql_stmt_fetch(this->stmt_looping) != 0 ){
		log_message(Log_Warning, "mysql_stmt_fetch failed: %s\n", mysql_stmt_error(this->stmt_looping));
	}

	mysql_stmt_free_result(this->stmt_looping);
	log_message(Log_Debug, "queue %d is%s looping\n", this->queue_id, this->loop_queue ? "" : " not");
	return 0;
}

void* module_alloc(){
	return malloc(sizeof(my));
}

int EXPORT module_init(my* this){
	/* setup function table (casting added because it expects a different
	 * pointer type (we are using an extended struct so it is compatible).*/
	this->module.next_slide   = (next_slide_callback)next_slide;
	this->module.queue_reload = (queue_reload_callback)queue_reload;
	//this->module.queue_dump   = (queue_dump_callback)queue_dump;
	this->module.queue_set    = (queue_set_callback)queue_set;

	/* initialize variables */
	this->loop_queue = 1;
	this->queue_id = 0;
	this->prev_slide_id = -1;
	this->conn = 0;
	this->stmt_slide  = 0;
	this->stmt_looping  = 0;
	this->stmt_pop = 0;

	return connect(this);
}

int EXPORT module_cleanup(my* this){
	/* it looks weird, but free_context only releases the fields not the
	 * pointer itself, so this is safe. */
	free_context(&this->module.context);

	/* "disconnect" database */
	return 0;//disconnect(this);
}
