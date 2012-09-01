required environment:
  linux, gcc, generated data

compile:
  gcc emsx.c -o emsx -lpthread

usage:
  ./emsx <infile> <outfile> [options]

options:
  -b <buffer-number>      set buffer number, default and maximum value is 130
  -db                     enable double buffering
  -p                      output 1kb pages (task 2)
  -c                      check correctness when sort completed (debug)
  -l <record-number>      sort first <record-number> records (debug)

example:
  ./emsx lineitem.tbl lineitem2.tbl        		(sort lineitem.tbl into lineitem2.tbl, using 130 buffer pages, without double buffering)
  ./emsx lineitem.tbl lineitem2.tbl -db    		(sort lineitem.tbl into lineitem2.tbl, using 130 buffer pages, with double buffering)
  ./emsx lineitem.tbl lineitem2.tbl -b 12       (sort lineitem.tbl into lineitem2.tbl, using 12 buffer pages, without double buffering)
  ./emsx lineitem.tbl lineitem2.tbl -b 12 -db   (sort lineitem.tbl into lineitem2.tbl, using 12 buffer pages, with double buffering)
  ./emsx lineitem.tbl lineitem2.tbl -p    		(sort lineitem.tbl into lineitem2.tbl, output 1kb pages)
  ./emsx lineitem.tbl lineitem2.tbl -l 1000 -c	(sort lineitem.tbl into lineitem2.tbl, sort first 1000 records, check for correctness)
