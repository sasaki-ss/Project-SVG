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

add_executable(interpreter_svg_element_interpreter_test
    interpreter/SvgElementInterpreterTest.cpp
    ../src/interpreter/SvgElementInterpreter.cpp
)

target_include_directories(interpreter_svg_element_interpreter_test
    PRIVATE
        ../src/interpreter
        ../src/parser
)

add_test(
    NAME interpreter.SvgElementInterpreter
    COMMAND interpreter_svg_element_interpreter_test
)
