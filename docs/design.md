
# Yalx Design

> Yalx = Yet another language X

## Types

Value types
Reference Types

### Primitive Types

All primitive types are value type.

|*Name*|Bits|
|------|----|
| `bool` | 8 |
| `int` | 32  | 
| `uint` | 32 |
| `i8` | 8 |
| `u8` | 8 |
| `i16` | 16 |
| `u16` | 16 |
| `i32` | 32 |
| `u32` | 32 |
| `i64` | 64 |
| `u64` | 64 |
| `f32` | 32 |
| `f64` | 64 |
| `char` | 32 |

Casting methods:

| Types |  `int`  |  `uint`  |  `i8`  |  `u8`  |  `i16`  |  `u16`  |  `i32`  |  `u32`  |  `i64`  |  `u64`  |  `f32`  |  `f64`  |
|-------|---------|----------|--------|--------|---------|---------|---------|---------|---------|---------|---------|---------|
| `int` |         | `toUint` | `toI8` | `toU8` | `toI16` | `toU16` | `toI32` | `toU32` | `toI64` | `toU64` | `toF32` | `toF64` |
| `uint`| `toInt` |          | `toI8` | `toU8` | `toI16` | `toU16` | `toI32` | `toU32` | `toI64` | `toU64` | `toF32` | `toF64` |
| `i8`  | `toInt` | `toUint` |        | `toU8` | `toI16` | `toU16` | `toI32` | `toU32` | `toI64` | `toU64` | `toF32` | `toF64` |
| `u8`  | `toInt` | `toUint` | `toI8` |        | `toI16` | `toU16` | `toI32` | `toU32` | `toI64` | `toU64` | `toF32` | `toF64` |
| `i16` | `toInt` | `toUint` | `toI8` | `toU8` |         | `toU16` | `toI32` | `toU32` | `toI64` | `toU64` | `toF32` | `toF64` |
| `u16` | `toInt` | `toUint` | `toI8` | `toU8` | `toI16` |         | `toI32` | `toU32` | `toI64` | `toU64` | `toF32` | `toF64` |
| `i32` | `toInt` | `toUint` | `toI8` | `toU8` | `toI16` | `toU16` |         | `toU32` | `toI64` | `toU64` | `toF32` | `toF64` |
| `u32` | `toInt` | `toUint` | `toI8` | `toU8` | `toI16` | `toU16` | `toI32` |         | `toI64` | `toU64` | `toF32` | `toF64` |
| `i64` | `toInt` | `toUint` | `toI8` | `toU8` | `toI16` | `toU16` | `toI32` | `toU32` |         | `toU64` | `toF32` | `toF64` |
| `u64` | `toInt` | `toUint` | `toI8` | `toU8` | `toI16` | `toU16` | `toI32` | `toU32` | `toI64` |         | `toF32` | `toF64` |
| `f32` | `toInt` | `toUint` | `toI8` | `toU8` | `toI16` | `toU16` | `toI32` | `toU32` | `toI64` | `toU64` |         | `toF64` |
| `f64` | `toInt` | `toUint` | `toI8` | `toU8` | `toI16` | `toU16` | `toI32` | `toU32` | `toI64` | `toU64` | `toF32` |         |

### Unit type

`unit'

### Array Types

`string`

Builtin Fields/Methods:

```
val length: int
toBytes(): u8[]
toChars(): char[]
charAt(i: int): char
```

Literal:

```
"string" '汉字' "😀==???" "$key=$value"
```

`T[N]` for example: `int[8]` `i8[32]` `u8[128]`

Literal:

```
{1, 2, 3} {"hello", "world", "demo"}
```

> Array can be value type or reference type

`array` Builtin Fields/Methods:

```
val size: int
```

### Annotation

```
annotation Value {
    id: int = 1 // Default value
    name: string
}

@Value(name = "${a}")
var a: int = 0
```

### Struct and Class

Struct type is value type and class type is reference type.

`struct`

```kotlin
struct Foo(
    val id: int,
    val name: string
) {
    private var foo: string
    private var bar: f32

    override fun toString() = "${this.foo}:${this.bar}"
}

val name = "name"
val foo = Foo(1, name)
foo.foo = "foo"
foo.bar = 1.1
val copiedFoo = foo // Deep copying
```

`class`

```kotlin
class Foo(val id: int, val name: string)
val foo = Foo(1, "name")
val bar = foo // Reference copying
```

`object`

```kotlin
object Foo {
    val i = 0
    val j = 1
    val k = 2
}

println(Foo.i, Foo.j)

```

`interface`

```kotlin
interface Foo {
    fun doIt(a: int, b: int): int
    fun doThis(a: int): int
    fun doThat(): string
}

class Bar {
    fun doIt(a: int, b: int) -> a + b
    fun doThis(a: int) -> a
    fun doThat() -> 'hello'
}

// Ducking type
val foo: Foo = Bar()
foo.doIt(1,2)
foo.doThis(1)
foo.doThat()
```

### Any

Then any type can be any value👍.

Definitions:

```kotlin

@Lang
class any {
    fun equals(other: any) -> any.id() == this.id()
    native fun id(): int
    native fun hashCode(): u32
    native fun toString(): string
    native fun isEmpty(): bool
}

```

### Function

```kotlin

fun foo(a: int, b: int) -> a + b
val functionObject = (a:int, b:int)->a+b
functionObject(1, 2)

fun bar(a: int, b: int): (int, string) {
    return a + b, "hello"
}

val (value, hint) = bar(1, 2)

```

### Generics

Function Generics:

```kotlin
fun foo<T>(a: T, b: T) = a + b

println(foo(1, 2))
println(foo(1.1, 2.2))
```

Class/Struct Generics

```kotlin
struct Foo<T>(
    val a: T,
    val b: int
)

val foo = Foo<string>("hello", 0)

class Map<K, V> {
    class Entry<K, V>(
        val key: K,
        val value: V
    ) {
        var next: Entry<K, V>
    }

    var entries: Entry<K, V>[] = empty
}

val items = Map<string, int>()
items.put("1st", 1)
items.put("2st", 2)


```

### No any nil/null value!

```kotlin

class Foo(
    private val id: int,
    private val name: string
) {
    override fun isEmpty() -> id == 0 && name == ""
}

// variable must has initialize value
val foo = Foo(1, 'jack')
assert(foo.isEmpty())

// '' is empty string
val bar = ''
assert(bar.isEmpty())

if (val foo = createFoo()) {
    // if foo !isEmpty()
} else {
    // if foo isEmpty()
}

```

## Source files

```kotlin
// Package declaration
package main

// Import statements
import testing.foo.bar.baz
import {
    testing.foo as Foo
    testing.bar as Bar
    testing.baz as *
}

// optioal: main function
fun main() {
    // entry:
    ...
}
```

## Syntax

### Literal Values

```
literal ::= integral_literal | floating_literal | boolean_literal | array_initializer
integral_literal ::= `-'? [0-9]+ (`L' | `l')?
                   | `0x' [0-9a-fA-F]+ (`L' | `l')?
floating_literal ::= `-' [0-9]* `.' [0-9]+ (`F' | `f')
                   | `-' [0-9]* `.' [0-9]+ (`E' | `e') ? [0-9]+
                   | `NaN'
boolean_literal := `true' | `false'
array_initializer ::= array_type? array_dimension_initializer
array_initializer_dimension ::= `{' `}'
                              | `{' expression? ( `,' expression )+ `}'
                              | `{' array_initializer_dimension? ( `,' array_initializer_dimension )+ `}'

lambda_literal ::= `(' argument_list `)' `->' expression
```

examples:

```
1  100l   -1   -100L   0x01  0xff
.1   1.1   2.2220f   2.332e12   -3.14e12

{1,2,3}  int[]{1,2,3}  u8[]{1,2,3} i64[8]{1,2,3}

int[2][2] {
    {1, 2},
    {3, 4}
}

int[2][2][2] {
    {
        {1, 2},
        {3, 4}
    }, {
        {1, 2},
        {3, 4}
    }
}

(a:int) -> a + 1
val r = ((a:int, b:int) -> a + b)(1,2)
assert(r == 3)
```

### Types

```
type_ref ::= `bool' | `i8' | `u8' | `i16' | `u16' | `i32' | `u32' | `i64' | `u64' | `f32' | `f64'
           | `int' | `uint' | `char' | `string'
           | symbol
           | generic_type
           | array_type
generic_type ::= symbol `<' type_list `>'
array_type ::= type_ref ( `[' ([0-9]+)? `]' ) +
function_type ::= `(' type_list? `)' (`->' type_ref | `(' type_list `)' )
symbol ::= identifier | identifier `.' identifier
```

examples:

```kotlin
int[] uint[2][2] string[1][2][3]

var: (int, int)->int = (a:int, b:int)->a+b
val: (string)->(int, int) = (a:string)->(1,2)
```

### Variable Declaration

```
variable_declaration ::= (`val' | `var') variable_name_type `=' expression_list
variable_name_type_list ::= variable_name_type ( `,' variable_name_type )*
variable_name_type ::= identifer (`:' type_ref)?
```

examples:

```kotlin
val a: int = 0
val b = 0
val c: i8 = -127
val d: u8 = 255
val e = 0L // e's type: i64
val f = ''
val g, h = 1, 2
```

```kotlin
var a: int = 0
a = 1
```

### Class/Struct/Object Definition

```
class_definition ::= `class' identifer generic_declaration? constructor? super_declaration? udt_block
struct_definition ::= `struct' identifer generic_declaration? constructor? super_declaration? udt_block
object_definition ::= `object' identifer udt_block
constructor ::= `(' constructor_member_definition* `)'
constructor_member_definition ::= annotation_declaration? access_description? (`val' | `var') identifer `:' type_ref
                                | argument
super_declaration ::= `:' symbol ( `(' expression_list? `)' )?
udt_block ::= `{' udt_item* `}'
udt_item ::= member_definition | method_definition
member_definition ::= annotation_declaration? access_description? `override' variable_declaration
method_definition ::= annotation_declaration? `override'?  access_description? ( function_definition | function_declaration)
access_description ::= `public' | `private' | `protected'
```

```kotlin
@Value(id=100, name='a')
class Foo(val id: int, val name: string, tags: u32) {
    @Json(name = 'flags')
    private val tags = tags & ~1

    fun getTags() -> this.tags

    // override from any::hashCode()
    override fun hashCode() -> this.tags
}

val foo = Foo(100, 'hello', 0xff)
assert(foo.id == 100)
assert(foo.name == 'hello')
assert(foo.getTags() == 0xff & ~1)

struct Bar(val x: int, val y: int)
val bar = Bar(1, 2)
val baz = bar // deep copy bar, bar is value type
```


### Interface Definition

```
interface_definition ::= `interface' identifer generic_declaration? udi_block
udi_block ::= `{' udi_item+ `}'
udi_item ::= identifier function_prototype
```

examples:

```kotlin
interface Foo {
    doIt(a: int, b: int): int
    doThat(a: string): (int, int)
    doThis(fmt: string, ...): string
}

// Template version
interface FooT<T> {
    doIt(a: T, b: T): T
}

// Comparable
interface Comparable<T> {
    int compareTo(a: T)
}

fun <T: Comparable<T>> sort(a: T[]) {
    ...
}

class Bar {
    fun doIt(a: int, b: int) -> a + b
    fun doThat(a: string) -> 1, 2
    fun doThis(fmt: string, ...) -> 'hello'
}

val foo: Foo = Bar() // Duck typing
assert(foo.doIt(1, 2) == 3)
val a, b = foo.doThat('')
assert(a == 1)
assert(b == 2)
assert(foo.doThis('') == 'hello')

val fooT: FooT<int> = Bar() // Use template version
```

### Function Definition

```
function_definition ::= `fun' generic_declaration? identifier function_prototype block
                      | `fun' generic_declaration? identifier `(' argument_list? `)' `->' expression
function_declaration ::= `native' `fun' identifier function_prototype
function_prototype ::= `(' argument_list? `)' ( `:'  return_types )?
return_types ::= type_ref | `(' type_list `)'
argument_list ::= argument+ ( `,' vargs )?
                 | vargs
argument ::= identifier `:' type_ref
vargs ::= `...'

generic_declaration ::= `<' generic_symbol_list `>'
generic_symbol_list ::= identifer ( `,' identifer )*
```

examples:

```kotlin
fun foo(a: int, b:int) -> a + b
fun bar(a: int, b:int): int {
    return a + b
}

// Template version
fun foo<T, P>(a: T, b: P) -> a + b
assert(foo(1, 2), 3)    // call foo<int, int> version
assert(foo(1L, 2L), 3L) // call foo<i64, i64> version

// External implements function
// No function body
native fun boo(a: int, b: int): int

// No return type aka. return type is unit
fun baz(a: int, b:int) {
    println("a=$a,b=$b")
}
// or
fun baz(a: int, b:int)->println("a=$a,b=$b")

// vargs
fun format(fmt: string, ...): string {
    // ...
}

```

### Annotation Decalaration

```
annotation_definition ::= `annotation' identifier `{' annotation_field* `}'
annotation_field ::= identifer `:' annotation_type ( `=' expression )
annotation_type ::= primitive_type | array_type | `string'

annotation_declaration ::= annotation_using+
annotation_using ::= `@' symbol ( `(' annotation_value_list `)' )?
annotation_value_list ::= annotation_value (`,' annotation_value)+
annotation_value ::= literal | `(' annotation_value_list `)'
```

examples:

```kotlin
annotation Value {
    id: int = 1 // defaults is `1'
    name: string
}

@Value(id = 100, name='aName')
var name: string = ''

annotation Method {
    id: int = 100
    name: string
    returnValue: Value
}

@Method(id=200, name='foo', returnValue=(id=100, name='unknown'))
fun foo(a: int, b: int) -> a + b
```

### Statements

```
block ::= `{' statement* `}'
statement ::= 
            | for_statement
            | while_statement
            | `run` call
            | `return' expression_list?
            | `break'
            | `continue'
```

While Statement

```
while_statement ::= `while' `(' condition `)' block
```

For Statement

```
for_statement ::= `for' `(' identifer `in' expression `)' block
                | `for' `(' identifer `in' expression `until' expression `)' block
                | `for' `(' identifer `in' expression `to' expression `)' block
```

### Expression