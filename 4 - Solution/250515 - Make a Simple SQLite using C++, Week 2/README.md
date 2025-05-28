# Assignment 1 - Solution

## Count rows in a table

Now that you've gotten your feet wet with the [SQLite database file format](https://www.sqlite.org/fileformat.html), it's time to move on to actual SQL!

### Task

I'll execute your program:

```bash
$ ./sqlite.exe sample.db "SELECT COUNT(*) FROM apples"
```

and here's the output it expects:

```bash
4
```

You'll need to read the table's row from the [sqlite_schema](https://www.sqlite.org/schematab.html) table and follow the rootpage value to visit the page corresponding to the table. For now you can assume that the contents of the table are small enough to fit inside the root page.

### Note

You don't need to implement a full-blown SQL parser just yet. We'll get to that in the next stages. For now you can just split the input by " " and pick the last item to get the table name.
