-- add timestamp to slide

-- sqlite seems to bail out if using CURRENT_TIMESTAMP in ALTER
ALTER TABLE `slide` ADD `timestamp` TIMESTAMP NOT NULL DEFAULT 0;
UPDATE `slide` SET `timestamp` = CURRENT_TIMESTAMP;
