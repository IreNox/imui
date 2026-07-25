find_package(Python COMPONENTS Interpreter REQUIRED)

execute_process(
	COMMAND ${Python_EXECUTABLE} -m pip install -r build-samples/_deps/glad-src/requirements.txt
)