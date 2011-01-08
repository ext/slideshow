PRAGMA foreign_keys = ON;
DROP TABLE IF EXISTS slide;
DROP TABLE IF EXISTS queue;
DROP TABLE IF EXISTS log;
DROP TABLE IF EXISTS user;

CREATE TABLE slide (
	id INTEGER PRIMARY KEY AUTOINCREMENT,           -- ID of the slide
	queue_id INTEGER NOT NULL REFERENCES queue(id), -- Queue ID the slide belong to
	path TEXT NOT NULL,                             -- Path to slide
	sortorder INTEGER NOT NULL DEFAULT 0,           -- Sort order index, ASC
	active TINYINT NOT NULL DEFAULT 1,              -- 1 if enabled, 0 if disabled
	assembler TEXT NOT NULL,                        -- Which assember is used to rasterize source data
	data BLOB NOT NULL
);

CREATE TABLE queue (
	id INTEGER PRIMARY KEY AUTOINCREMENT,          -- ID of queue
	name TEXT NOT NULL,                            -- Name of queue
	loop TINYINT NOT NULL DEFAULT 1                -- 1 if looping is enabled, 0 if disabled.
);

CREATE TABLE log (
       id INTEGER PRIMARY KEY AUTOINCREMENT,
       type INTEGER NOT NULL,
       severity INTEGER NOT NULL,
       stamp TIMESTAMP NOT NULL,
       user_id INTEGER NOT NULL DEFAULT 1 REFERENCES user(id),
       message TEXT NOT NULL
);

-- User table
-- user/host tuple is used for matching, a host may be specified instead of a username or even both.
CREATE TABLE user (
       id INTEGER PRIMARY KEY AUTOINCREMENT,
       name TEXT,
       host TEXT
);

INSERT INTO queue (id, name) VALUES (-1, 'Intermediate');
INSERT INTO queue (id, name) VALUES ( 0, 'Unsorted');
INSERT INTO queue (id, name) VALUES ( 1, 'Default queue');
INSERT INTO queue (name) VALUES ('test');
INSERT INTO user (name, host) VALUES ('daemon', NULL);
