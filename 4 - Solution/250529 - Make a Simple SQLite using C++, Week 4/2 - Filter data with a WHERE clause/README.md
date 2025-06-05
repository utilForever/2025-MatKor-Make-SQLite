# Assignment 2 - Solution

## 1 - Read data from multiple columns

This assignment is similar to the previous lecture, just that the tester will query for multiple columns instead of just one.

### Task

I'll execute your program:

```bash
$ ./sqlite.exe sample.db "SELECT name, color FROM apples"
```

and here's the output it expects:

```bash
Granny Smith|Light Green
Fuji|Red
Honeycrisp|Blush Red
Golden Delicious|Yellow
```

The order of rows doesn't matter.

## 2 - Filter data with a WHERE clause

In this assignment, you'll support filtering records using a WHERE clause.

### Task

I'll execute your program:

```bash
$ ./sqlite.exe sample.db "SELECT name, color FROM apples WHERE color = 'Yellow'"
```

and here's the output it expects:

```bash
Golden Delicious|Yellow
```

For now you can assume that the contents of the table are small enough to fit inside the root page. We'll deal with tables that span multiple pages in the next lexture.
