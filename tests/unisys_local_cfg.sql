-- 添加本地默认配置

TRUNCATE TABLE device;
TRUNCATE TABLE alarm;
TRUNCATE TABLE action;
BEGIN;

INSERT INTO device(type, ip, port, name) VALUES(0x01000001, '1.1.0.0', 0, 'UNS');
INSERT INTO device(type, ip, port, name, attach_id) VALUES(0x01000002, '127.0.0.1', 5850, 'CMS', 1);
INSERT INTO device(type, ip, port, name, attach_id) VALUES(0x01000003, '127.0.0.1', 5860, 'DAG', 2);
INSERT INTO device(type, ip, port, name, attach_id) VALUES(0x01000004, '127.0.0.1', 5870, 'EMS', 2);
INSERT INTO device(type, ip, port, name, attach_id) VALUES(0x01000005, '127.0.0.1', 5880, 'ACS', 2);
INSERT INTO device(type, ip, port, name, attach_id) VALUES(0x01000006, '127.0.0.1', 5890, 'DPS', 2);
-- dev:7 模拟器
INSERT INTO device(type, ip, port, name, attach_id) VALUES(0x02030001, '127.0.0.1', 5800, 'SIMU', 3);


-- CREATE TABLE alarm(
-- id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL comment '告警ID',
-- type INT UNSIGNED NOT NULL comment '告警类型',
-- dev_id INT UNSIGNED NOT NULL comment '设备ID',
-- channel INT UNSIGNED comment '告警通道'
-- )comment '告警配置表';

-- alarm:1 关联dev: 7 io告警 通道1
INSERT INTO alarm(type, dev_id, channel) VALUES(0x03010001, 7, 1);
-- alarm:2 关联dev: 7 io告警 通道2
INSERT INTO alarm(type, dev_id, channel) VALUES(0x03010001, 7, 2);
-- alarm:3 关联dev: 7 异常状态告警
INSERT INTO alarm(type, dev_id) VALUES(0x03020003, 7);

-- CREATE TABLE action(
-- id INT UNSIGNED auto_increment PRIMARY KEY NOT NULL comment '联动ID',
-- type INT UNSIGNED NOT NULL comment '联动类型',
-- alarm_id INT UNSIGNED NOT NULL comment '关联的告警ID',
-- detail BLOB comment '联动详情'
-- )comment '联动配置表';

-- action:1 关联alarm:1 IO联动 通道号在detail,(未配置通道号打印日子后忽略)
INSERT INTO action(type, alarm_id) VALUES(0x04000001, 1);
-- actino:2 关联alarm:2 PTZ联动
INSERT INTO action(type, alarm_id) VALUES(0x04000005, 2);
-- actino:3 关联alarm:2 TTS联动
INSERT INTO action(type, alarm_id) VALUES(0x04000002, 3);

COMMIT;
