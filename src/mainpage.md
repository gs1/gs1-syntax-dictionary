Main Page {#mainpage}
=========

The GS1 Barcode Syntax Dictionary contains entries describing the composition
of each Application Identifier (AI) in the GS1 system in terms of its
concatenated components.

Each component may be associated with one or more GS1 Barcode Syntax Tests
("linters") whose task is to analyse the component and detect common errors
using the information that is available to them.

This documentation describes the actions of the set of reference linters that
are provided by GS1 for use with the GS1 Barcode Syntax Dictionary, which are
available here:

<https://ref.gs1.org/tools/gs1-barcode-syntax-resource/syntax-tests/implementation/>


@remark

For each target platform it is intended that the linters may be cross-compiled,
wrapped with language-specific code that calls the native functions,
transliterated, or otherwise made available.

For each linter, its general action is described and a detailed reference is
provided in the form of annotated C language source.

If the functions are being transliterated then the resulting code should be
cross checked by additionally porting the extensive unit tests provided in the
reference linter source files.


### Linter reference

!!!REFERENCE!!!
