Record manager
==============


Offsets of the bit-fields in an old-style record. NOTE! In the table the
most significant bytes and bits are written below less significant.

        (1) byte offset		(2) bit usage within byte
        downward from
        origin ->	1	8 bits pointer to next record
                        2	8 bits pointer to next record
                        3	1 bit short flag
                                7 bits number of fields
                        4	3 bits number of fields
                                5 bits heap number
                        5	8 bits heap number
                        6	4 bits n_owned
                                4 bits info bits


Offsets of the bit-fields in a new-style record. NOTE! In the table the
most significant bytes and bits are written below less significant.

        (1) byte offset		(2) bit usage within byte
        downward from
        origin ->	1	8 bits relative offset of next record
                        2	8 bits relative offset of next record
                                  the relative offset is an unsigned 16-bit
                                  integer:
                                  (offset_of_next_record
                                   - offset_of_this_record) mod 64Ki,
                                  where mod is the modulo as a non-negative
                                  number;
                                  we can calculate the offset of the next
                                  record with the formula:
                                  relative_offset + offset_of_this_record
                                  mod UNIV_PAGE_SIZE
                        3	3 bits status:
                                        000=conventional record
                                        001=node pointer record (inside B-tree)
                                        010=infimum record
                                        011=supremum record
                                        1xx=reserved
                                5 bits heap number
                        4	8 bits heap number
                        5	4 bits n_owned
                                4 bits info bits






