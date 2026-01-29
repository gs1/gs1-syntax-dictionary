GS1 Barcode Syntax Dictionary and Syntax Tests
==============================================

The **GS1 Barcode Syntax Dictionary** ("Syntax Dictionary") is a text file that is both human-readable and
machine-readable, which consists of a set of entries describing each currently
assigned GS1 Application Identifier in terms of its constituent components for
the purpose of input validation.

The current revision of the Syntax Dictionary can be viewed and downloaded
from here:

<https://ref.gs1.org/tools/gs1-barcode-syntax-resource/syntax-dictionary/>

The contents of the dictionary are intentionally straightforward, however it is
sufficient to facilitate certain activities that are essential for processing
GS1 Application Identifier and GS1 Digital Link data, chiefly:

  * Accurately convert between the various different representations of GS1 Application Identifier syntax data.
    * Bracketed and unbracketed format
    * Barcode message scan data
    * HRI and non-HRI text
    * GS1 Digital Link URIs
  * Validate Application Identifier associations, in particular mutually-exclusive AIs and requisite AIs.

The **GS1 Barcode Syntax Tests** ("Linters") are referred to by the
AI entries within the Syntax Dictionary, and enable the user to perform
validation of the syntax of the content for AI-based messages such as AI
element strings and GS1 Digital Link URIs. Reference implementations of the
routines are provided in the C language.

The Syntax Dictionary, together with the Linters, can either be used directly
or transliterated into third-party code. It is intended that it should be
straightforward for projects that adopt these resources to update to new
revisions whenever there are changes to the Syntax Dictionary and new Linters
in response to updates in the corresponding specifications.

Further details about the activities and motivation for the Syntax Dictionary
are given in the following article:

<https://www.linkedin.com/pulse/gs1-application-identifier-syntax-dictionary-terry-burton/>

This repository contains the following key artifacts:

| Artifact                    | Purpose                                                                                                              |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `gs1-syntax-dictionary.txt` | The Syntax Dictionary                                                                                                |
| `src/lint_<name>.c`         | Source for the reference Linters, which includes unit tests                                                          |
| `src/gs1syntaxdictionary.h` | Headers file with Linter function declarations and Linter error code definitions                                     |
| `src/gs1syntaxdictionary.c` | Optional implementations for mapping Linter names to functions and Linter error codes to error message strings       |
| `docs/`                     | Linter function descriptions in HTML format                                                                          |


Documentation
-------------

The structure of the Syntax Dictionary is defined in detail by its introductory comments.

The reference Linters are extensively documented in the code, with the documentation being
made available online here:

<https://ref.gs1.org/tools/gs1-barcode-syntax-resource/syntax-tests/>


Using the Syntax Dictionary and Linters
---------------------------------------

The software license for this project is permissive allowing for the source code to be vendored into a
codebase (Open Source or proprietary) and compiled into an application, or for a
pre-built shared library of Linter routines to be redistributed (either
alongside a third-party application or provided separately) and dynamically loaded at
runtime.

Applications that wish to implement the Syntax Dictionary and Linters must
provide the necessary framework code. This includes parsing the Syntax
Dictionary to initialise any internal data structures, then using the
extracted rules to validate and transform AI-based messages such as AI element
strings or GS1 Digital Link URIs.

For example, depending on the requirements of the application it may need to:

  * Apply the format specification rules for AIs to separate the AI element string or GS1 Digital Link URI message into distinct AIs, e.g. using FNC1 separators or predefined fixed-length.
  * Separate AIs into parts based on their components' designated length.
  * Apply the format specification and Linters (or a port / translation of them) to the AI components to validate their contents.
  * Apply the AI association rules over the entirety of the AI data to validate exclusive and mandatory AI pairings and/or GS1 Digital Link path primary-key to key-qualifier associations.
  * Construct valid GS1 barcode message data (i.e. with FNC1 in first), bracketed/unbracketed element strings, HRI/non-HRI text, and GS1 Digital Link URIs based on the AI component format specifications and AI associations.

The **GS1 Syntax Engine** is a library that provides one such framework
implementation of the Syntax Dictionary and Linters, and serves as an example
of how to use this project effectively:

<https://ref.gs1.org/tools/gs1-barcode-syntax-resource/syntax-engine/>

The GS1 Syntax Engine builds the Linter routines into its library code and
demonstrates two distinct approaches to integrating with the Syntax Dictionary:
(1) Processing the Syntax Dictionary to dynamically populate internal data
structures during application startup; (2) embedding a static table of data
derived from the Syntax Dictionary during the application build process, that
is used as a fallback if the Syntax Dictionary file is not available.

Either of these approaches may be used to enhance the capabilities of an
application to facilitate transformation and validation of GS1 AI-based data.

Alternatively developers may choose to program their application to use the
Syntax Engine library directly (by vendoring in the source or linking the C
library, possibly using one of the language bindings) to entirely avoid
implementing the Syntax Dictionary and Linter framework themselves.


### Building the Linters on Windows

The Linters can be rebuilt on Windows using MSVC.

The project contains a solution file (.sln) compatible with recent versions of
Microsoft Visual Studio. In the Visual Studio Installer you will need to ensure
that MSVC is installed by selecting the "C++ workload".

Alternatively, they can be built from the command line by opening a
Developer Command Prompt, cloning this repository, changing to the `src`
directory and building the solution using:

    msbuild /p:Configuration=release gs1syntaxdictionary.sln

Or:

    msbuild /p:Configuration=debug gs1syntaxdictionary.sln


### Building the Linters on Linux or MacOS

The Linters can be rebuilt on any Linux or MacOS system that has a C compiler
(such as GCC or Clang).

To build using the default compiler change into the `src` directory and run:

    make

A specific compiler can be chosen by setting the CC argument for example:

    make CC=gcc

    make CC=clang

There are a number of other targets that are useful for development purposes:

    make test [SANITIZE=yes]  # Run the unit test suite, optionally building using LLVM sanitizers
    make fuzzer               # Build fuzzers for exercising the individual Linters. Requires LLVM libfuzzer.

