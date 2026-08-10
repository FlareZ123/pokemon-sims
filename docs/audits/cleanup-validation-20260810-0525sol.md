# Cleanup validation marker

This PR exists only to run the repository's pull-request CI against the cleanup composition tree at `4f539a7a296b9c63cb5cea44d80626eca5878d93`.

The production changes being validated are already on `main`:

- `ab9b6eeb0677206bf28674e57809663383904342` centralizes the remaining legacy search bridge includes.
- `ff57521f2f219c4de33abb799128459a5c97b19f` composes that bridge through the late search wrapper.
- `4f539a7a296b9c63cb5cea44d80626eca5878d93` simplifies the final composition include guard.

C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
