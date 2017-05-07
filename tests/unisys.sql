-- create database unisys;
-- grant all on unisys.* to unsclient@'localhost' identified by 'unsclient';
-- create table device();
CREATE TABLE device(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL,
type INT UNSIGNED NOT NULL,
attr BLOB
);

CREATE TABLE alarm_ability(
dev_type INT UNSIGNED NOT NULL,
alarm_type INT UNSIGNED NOT NULL,
name VARCHAR(64),
num INT UNSIGNED
);
CREATE TABLE alarm(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL,
type INT UNSIGNED NOT NULL,
dev_id INT UNSIGNED NOT NUL,
detail BLOB
);

CREATE TABLE action(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL,
type INT UNSIGNED NOT NULL,
alarm_id INT UNSIGNED NOT NUL,
detail BLOB --dev_ids
);

CREATE TABLE alram_log(
id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL,
type INT UNSIGNED NOT NULL,
dev_id INT UNSIGNED NOT NULL,
title VARCHAR(128) NOT NULL,
level INT NOT NULL,
start DATETIME,
stop DATETIME,
detail VARCHAR(512),
INDEX idx_alarm_start(start),
INDEX idx_alarm_title(title)
);
