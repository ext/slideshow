PRAGMA foreign_keys = ON;
DROP TABLE IF EXISTS slide;
DROP TABLE IF EXISTS queue;

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
	name TEXT NOT NULL                             -- Name of queue
);

INSERT INTO queue (name) VALUES ('Default queue');
INSERT INTO queue (name) VALUES ('test');
INSERT INTO slide (queue_id, path, assembler, data) VALUES (1, 'test1.slide', 'image', 'foo.png');
INSERT INTO slide (queue_id, path, assembler, data) VALUES (1, 'test2.slide', 'image', 'bar.png');
INSERT INTO slide (queue_id, path, assembler, data) VALUES (2, 'test3.slide', 'image', 'baz.png');
