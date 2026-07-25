find_package(Python3 COMPONENTS Interpreter REQUIRED)

execute_process(
	COMMAND python -m pip install -r build-samples/_deps/glad-src/requirements.txt
)