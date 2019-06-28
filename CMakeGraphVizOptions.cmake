SET(GRAPHVIZ_EXTERNAL_LIBS NO)

#
# The list of tragets ignored when dependency graph is generated
#

set(GRAPHVIZ_IGNORE_TARGETS "")
string(APPEND GRAPHVIZ_IGNORE_TARGETS "/usr/include/*")
# string(APPEND GRAPHVIZ_IGNORE_TARGETS "|test_*")


SET(GRAPHVIZ_EXECUTABLES NO)

