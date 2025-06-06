# Assignment 3

## Retrieve data using an index

In this assignment, we'll implement an index scan. Rather than reading all rows in a table and then filtering in-memory, we'll use an index to perform a more intelligent search.

The test database contains a `companies` table with an index named `idx_companies_country` on the `country` column.

### Task

I'll execute your program:

```bash
$ ./sqlite.exe companies.db "SELECT id, name FROM companies WHERE country = 'eritrea'"
```

and here's the output it expects:

```bash
121311|unilink s.c.
2102438|orange asmara it solutions
5729848|zara mining share company
6634629|asmara rental
```

To test whether your implementation actually uses an index, the tester will use a database is ~1GB in size and expect your program to return query results in less than 3 seconds. You can assume that all queries run by the tester will include `country` in the `WHERE` clause, so they can be served by the index. The tester will run multiple randomized queries and expect all of them to return results in under 3 seconds.
