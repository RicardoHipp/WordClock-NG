# ArduinoJson - https://arduinojson.org
# Copyright © 2014-2026, Benoit BLANCHON
# MIT License

add_executable(MemoryPoolTests
	allocVariant.cpp
	clear.cpp
	saveString.cpp
	size.cpp
	StringCopier.cpp
)

add_test(MemoryPool MemoryPoolTests)

set_tests_properties(MemoryPool
	PROPERTIES
		LABELS "Catch"
)
