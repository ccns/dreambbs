PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE brd (
  id integer NOT NULL primary key autoincrement,
  brd text NOT NULL
);
CREATE TABLE counter (
  BID integer NOT NULL,
  PID text NOT NULL ,
  counter integer NOT NULL DEFAULT (0)
);
COMMIT;
