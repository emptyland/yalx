# Language Builtin Libraries

## Class `any`

```kotlin

package lang // yalx.lang

@Lang
class any {
    native fun id(): int
    native fun hashCode(): u32
    native fun toString(): string
    native fun isEmpty(): bool
    fun finalize() -> Unit
}

```

## Comparable

```kotlin

@Lang
interface Comparable<T> {
    // Test two objects equals
    equals(other: T): bool

    // Compare two objects
    compareTo(other: T): int
}

```

examples:

```kotlin
val shell1 = 1 as Comparable<int> // int type implements Comparable
```