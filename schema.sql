DROP TABLE IF EXISTS slide;
DROP TABLE IF EXISTS queue;

CREATE TABLE slide (
       id INTEGER PRIMARY KEY AUTOINCREMENT,           -- ID of the slide
       queue_id INTEGER NOT NULL REFERENCES feed(id),  -- Queue ID the slide belong to
       path TEXT NOT NULL,                             -- Full path to the rasterized image
       sortorder INTEGER NOT NULL DEFAULT 0,           -- Sort order index, ASC
       active TINYINT NOT NULL DEFAULT 1,              -- 1 if enabled, 0 if disabled
       assembler TEXT NOT NULL,                        -- Which assember is used to rasterize source data
       data BLOB NOT NULL
);

CREATE TABLE queue (
       id INTEGER PRIMARY KEY AUTOINCREMENT,          -- ID of queue
       name TEXT NOT NULL                             -- Name of queue
);
