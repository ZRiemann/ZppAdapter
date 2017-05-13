CREATE DATABASE unisys;

use unisys;
-- grant all on unisys.* to unsclient@'localhost' identified by 'unsclient';
-- create table device();
CREATE TABLE device(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL comment '设备唯一ID',
type INT UNSIGNED NOT NULL comment '设备类型',
ip VARCHAR(64) comment '设备IPv4/IPv6',
port INT UNSIGNED comment '设备应答端口',
name VARCHAR(64) CHARSET utf8 comment '设备名称',
attach_id INT UNSIGNED comment '关联设备ID',
attr BLOB comment '设备属性'
)comment '设备总表';

CREATE TABLE alarm_ability(
dev_type INT UNSIGNED NOT NULL comment '设备类型',
alarm_type INT UNSIGNED NOT NULL comment '告警类型',
name VARCHAR(64) comment '告警名称',
channels INT UNSIGNED comment '告警通道数'
)comment '告警能力集';

CREATE TABLE alarm(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL comment '告警ID',
type INT UNSIGNED NOT NULL comment '告警类型',
dev_id INT UNSIGNED NOT NULL comment '设备ID',
channel INT UNSIGNED comment '告警通道'
)comment '告警配置表';

CREATE TABLE action(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL comment '联动ID',
type INT UNSIGNED NOT NULL comment '联动类型',
alarm_id INT UNSIGNED NOT NULL comment '关联的告警ID',
detail BLOB comment '联动详情'
)comment '联动配置表';

CREATE TABLE alram_log(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL comment '自增ID',
type INT UNSIGNED NOT NULL comment '告警类型',
dev_id INT UNSIGNED NOT NULL comment '设备ID',
channel INT UNSIGNED comment '设备通道',
title VARCHAR(128) NOT NULL comment '简情',
level INT comment '告警等级',
start DATETIME comment '开始时间',
stop DATETIME comment '结束时间',
detail VARCHAR(512) comment '详情描述',
INDEX idx_alarm_start(start),
INDEX idx_alarm_title(title)
)comment '告警日志';
