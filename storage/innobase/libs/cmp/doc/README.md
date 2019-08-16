Comparison services for records
===============================


ALPHABETICAL ORDER
==================

The records are put into alphabetical order in the following
way: let F be the first field where two records disagree.
If there is a character in some position n where the
records disagree, the order is determined by comparison of
the characters at position n, possibly after
collating transformation. If there is no such character,
but the corresponding fields have different lengths, then
if the data type of the fields is paddable,
shorter field is padded with a padding character. If the
data type is not paddable, longer field is considered greater.
Finally, the SQL null is bigger than any other value.

At the present, the comparison functions return 0 in the case,
where two records disagree only in the way that one
has more fields than the other.
