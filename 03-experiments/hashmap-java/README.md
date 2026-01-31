# HashMap with Array Chaining

Two implementations of a chaining-based HashMap in Java 11.

## Implementations

| Class | Description |
|-------|-------------|
| `ArrayListHashMap` | Uses Java's built-in `ArrayList` for buckets |
| `DynamicArrayHashMap` | Uses hand-rolled `DynamicArray` for buckets |

## Features

- `put(key, value)` - Insert or update
- `get(key)` - Retrieve value
- `remove(key)` - Delete entry
- `containsKey(key)` - Check existence
- Auto-resize when load factor > 0.75

## Run

```bash
javac *.java
java HashMapTest
```
