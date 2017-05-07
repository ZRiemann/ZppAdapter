-- 添加本地默认配置
BEGIN;
INSERT INTO device(type, ip, port, name) VALUES(0x01010000, '127.0.0.1', 5858, 'CMS中文');

COMMIT;
