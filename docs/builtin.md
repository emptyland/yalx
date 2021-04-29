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

## Number

```kotlin
@Lang
interface Numeric {
    toI8(): i8
    toU8(): u8
    toI16(): i16
    toU16(): u16
    toI32(): i32
    toU32(): u32
    toInt(): int
    toUInt(): uint
    toI64(): i64
    toU64(): u64
    toF32(): f32
    toF64(): f64
}

@Lang
class Number<T: Numeric> {
    native fun parse(s: string): T

    equals(other: T) -> this.value == other

    compareTo(other: T) -> when {
        this.value < other -> -1
        this.value > other -> 1
        else -> 0
    }
}

```

examples:

```kotlin
val n = i8.parse('127')
// native symbol is: int yalx_lang_i8_parse_s_i8(yalx_str_t *s)

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