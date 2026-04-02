add_executable(parser_xml_node_extractor_test
    parser/XmlNodeExtractorTest.cpp
    ../src/parser/XmlNodeExtractor.cpp
)

target_include_directories(parser_xml_node_extractor_test
    PRIVATE
        ../src/parser
)

add_test(
    NAME parser.XmlNodeExtractor
    COMMAND parser_xml_node_extractor_test
)
