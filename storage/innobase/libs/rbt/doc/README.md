Definition of a red-black tree
==============================

 A red-black tree is a binary search tree which has the following
 red-black properties:

    1. Every node is either red or black.
    2. Every leaf (NULL - in our case tree->nil) is black.
    3. If a node is red, then both its children are black.
    4. Every simple path from a node to a descendant leaf contains the
       same number of black nodes.

    from (3) above, the implication is that on any path from the root
    to a leaf, red nodes must not be adjacent.

    However, any number of black nodes may appear in a sequence.


