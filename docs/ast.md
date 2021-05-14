# AST and IL Nodes

## Node Tree

```
Node
    + ast::
    +- FileUnit
    +- Statement
        +- Declaration
            +- VariableDeclaration
            +- FunctionDeclaration
            +- ObjectDeclaration
        +- Definition 
            +- InterfaceDefinition
            +- ClassDefinition
        +- Expression
            +- Identifier
            +- Literal
                +- UnitLiteral
                +- ActualLiteral<T>
    +- Type
        +- ArrayType
        +- ClassType
        +- FunctionPrototype
```


## Base Node

* Class Node is arean object.
* Only allocate from arean.
* All AST/IL nodes base class

```
class Node
    = Accept(NodeVisitor*): Visitation // Accept visitor
    = IsXXX(): bool // Type checker
    ...
    = AsXXX(): XXX* // Type casting
    ...
    + kind: Kind // Kind of node
    + source_position: SourcePosition // Source file position
```

```
class SourcePosition
    + begin_column: int
    + begin_line: int
    + end_column: int
    + end_line: int
```

## AST Nodes

```
class FileUnit : Node
    + file_name: NString *
    + file_full_path: NString *
    + package_name: NString *
    + imports: List<ImportEntry> *
    + statements: List<Statement *> *
    + definitions: List<Definition *> *
    + declarations: List<Declaration *> *
```

```
class ImportEntry : Node
    + original_package_name: NString *
    + package_path: NString *
    + alias: NString *
```

```
class Declaration : Statement
    = Identifier(): Identifer *
    = Type(): TypeRef *
    = Item(int i): Declaration *
    = ItemSize(): size_t
    + access: {kExport, kPublic, kProtected, kPrivate, kDefault}
```

```
class VariableDeclaration : Declaration
    + constraint: {kVal, kVar}
    + is_volatile: bool
    + variables: List<Item *>
    class Item : Declaration
        + identifer: Identifer *
        + type: Type *
    + initilaizers: List<Expression *>
```

```
class FunctionDeclaration : Declaration
    + name: NString *
    + decoration: {kNative, kAbstract, kOverride}
    + define_for: Definition *
    + prototype: FunctionPrototype *
    + reduece: Expression * // 'fun foo() ->' syntax
    + body: Block * // 'fun foo(): xxx {...}' syntax
```

```
class ObjectDeclaration : Declaration
    + name: NString *
    + methods: List<FunctionDeclaration *>
    + members: List<VariableDeclaration *>
```

```
class Definition : Statement
    + generic_params: List<GenericParameter *>
    + has_instantiated: bool
```

```
class GenericParameter : Node
    = Identifier(): Symbol *
    + identifier: Type *
    + constraint: Type *
```

```
class InterfaceDefinition : Definition
    + name: NString *
    + methods: List<FunctionDeclaration *>
```

```
class ClassDefinition : Definition
    + name: NString *
    + constructor: List<VariableDeclaration *> // class Foo(val i: int, j: int) {}
    + base: Type *
    + super_calling: List<Expression *>
    + methods: List<FunctionDeclaration *>
    + members: List<VariableDeclaration *>
```

```
class Symbol : Node
    + prefix_name: NString *
    + name: NString *
```

### Expression

```
class Identifer : Expression
    + name: NString *
```

```
class Literal : Expression
    = Type(type: Type *): Type *
    + type: Type *
```

```
class UnitLiteral : Literal
```

```
class ActualLiteral<T> : Literal
    + value: T

IntLiteral  <- ActualLiteral<int>
UIntLiteral <- ActualLiteral<unsigned>
I64Literal  <- ActualLiteral<int64_t>
U64Literal  <- ActualLiteral<uint64_t>
BoolLiteral <- ActualLiteral<bool>
StringLiteral <- ActualLiteral<NString *>
```

```
class ArrayInitializer : Expression
    = dimension_count(): int
    + hint_type: ArrayType *
    + dimension: List<ArrayDimension *>

class ArrayDimension : Expression
    + values: List<ArrayDimension *| Expression *>
```


### Type Reference

```
class Type : Node
    = Sign(buf: char *, n: size_t)
    = IsArray(): bool
    = IsClass(): bool
    = IsFunction(): bool
    = AsArray(): ArrayType *
    = AsClass(): ClassType *
    = AsFunction(): FunctionPrototype *
    = ToArray(dimension_count: int): ArrayType *
    = ToClass(class_def: ClassDefinition *): ClassType *
    = ToFunction(...): FunctionPrototype *
    + primary_type: {t_unit, t_bool, t_i8, t_u8, t_i16, ..., t_string, t_chan, t_fun, t_class, t_identifier, t_array}
    + identifier: Symbol *
    + generic_args: List<Type *>
```

```
class ArrayType : Type
    + dimension_count: int
```

```
class ClassType: Type
    + class_def: ClassDefinition *
```

```
class InterfaceType: Type
    + interface_def: InterfaceDefinition *
```

```
class FunctionPrototype : Type
    + parameters: List<VariableDeclaration::Item *>
    + vargs: bool
    + returns: List<Type *>
```