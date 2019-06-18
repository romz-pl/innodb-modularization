#
# Configuration file for CMake generating graphviz files
#

#
# The graph type
#
SET(GRAPHVIZ_GRAPH_TYPE "digraph")

#
# The graph name.
#
SET(GRAPHVIZ_GRAPH_NAME "Dependency graph")

#
# The header written at the top of the graphviz file.
#
SET(GRAPHVIZ_GRAPH_HEADER "node [ fontsize = \"20\" ];")

#
# Set this to FALSE to exclude external libraries from the generated graphs.
#
SET(GRAPHVIZ_EXTERNAL_LIBS FALSE)

#
# Set this to FALSE to exclude executables from the generated graphs.
#
SET(GRAPHVIZ_EXECUTABLES FALSE)


#
# Set this to FALSE to exclude per target graphs foo.dot.<target>.
#
SET(GRAPHVIZ_GENERATE_PER_TARGET FALSE)

#
# Set this to FALSE to exclude depender graphs foo.dot.<target>.dependers.
#
SET(GRAPHVIZ_GENERATE_DEPENDERS FALSE)

#
# The list of tragets ignored when dependency graph is generated
#
set(GRAPHVIZ_IGNORE_TARGETS "")
string(APPEND GRAPHVIZ_IGNORE_TARGETS "gmock*")
string(APPEND GRAPHVIZ_IGNORE_TARGETS "|gtest*")

# see the pipe sign |
#string(APPEND GRAPHVIZ_IGNORE_TARGETS "|not-needed-library")




