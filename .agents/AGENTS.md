# Rule: Doxygen Updates
Whenever you refactor, rename, or modify a class, struct, function, or variable, you MUST immediately update its accompanying Doxygen comment (`/// @brief`, `@param`, `@return`) in the header file during the same turn.

# Rule: Always Run Tests
Whenever you write, refactor, or modify code, you MUST always compile and run the project's test suite to verify your changes haven't broken anything. 
- To build the tests: `cmake --build build --target GravityBilliardsTests`
- To run the tests: `cd build; ctest --output-on-failure`

# Rule: Test-Driven Features & Manual Math Verification
Whenever you add new features, you MUST write accompanying unit tests. If the features involve mathematical calculations or physics logic, you must manually "think" the results through (calculate expected values by hand or with a calculator) rather than just copy-pasting the runtime formula into the test. Write out your manual math steps in a comment above the assertion!
