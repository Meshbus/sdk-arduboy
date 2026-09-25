#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Generate the supported Arduino Sketch subset using tokens and balanced scopes.

This is deliberately not a C++ parser. Ambiguous function declarators, templates,
conditional definitions and namespace/linkage blocks require ordinary C++ sources.
No function is discovered by a regular-expression match over source text.
"""
import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import re

class SketchError(ValueError):
    pass

@dataclass
class Token:
    text: str
    start: int
    end: int
    line: int
    conditional: int
    kind: str = 'token'


def tokenize(source, path):
    tokens = []
    i = 0
    line = 1
    conditional = 0
    while i < len(source):
        start, start_line = i, line
        ch = source[i]
        if ch.isspace():
            line += ch == '\n'; i += 1; continue
        if source.startswith('//', i):
            end = source.find('\n', i)
            i = len(source) if end < 0 else end
            continue
        if source.startswith('/*', i):
            end = source.find('*/', i + 2)
            if end < 0: raise SketchError(f'{path}:{line}: unterminated comment')
            i = end + 2; line += source[start:i].count('\n'); continue
        if ch == '#' and not source[source.rfind('\n', 0, i)+1:i].strip():
            while True:
                end = source.find('\n', i)
                if end < 0: i = len(source); break
                i = end + 1
                if not source[start:end].rstrip().endswith('\\'): break
            text = source[start:i]
            directive = text[1:].lstrip().split(None, 1)[0]
            if directive in ('if', 'ifdef', 'ifndef'): conditional += 1
            elif directive == 'endif':
                conditional -= 1
                if conditional < 0: raise SketchError(f'{path}:{line}: unmatched #endif')
            tokens.append(Token(text, start, i, line, conditional, 'directive'))
            line += text.count('\n'); continue
        raw = re.match(r'(?:u8|u|U|L)?R"([^ ()\\\t\r\n]{0,16})\(', source[i:])
        if raw:
            close = ')' + raw.group(1) + '"'
            end = source.find(close, i + len(raw.group(0)))
            if end < 0: raise SketchError(f'{path}:{line}: unterminated raw string')
            i = end + len(close)
            kind = 'literal'
        elif ch in ('"', "'"):
            i += 1
            while i < len(source) and source[i] != ch:
                if source[i] == '\\': i += 2
                else: i += 1
            if i >= len(source): raise SketchError(f'{path}:{line}: unterminated literal')
            i += 1; kind = 'literal'
        elif ch.isalpha() or ch == '_':
            i += 1
            while i < len(source) and (source[i].isalnum() or source[i] == '_'): i += 1
            kind = 'identifier'
        else:
            i += 1; kind = 'token'
        tokens.append(Token(source[start:i], start, i, start_line, conditional, kind))
        line += source[start:i].count('\n')
    if conditional: raise SketchError(f'{path}:{line}: unterminated conditional')
    return tokens


def function_signature(tokens, path, definition):
    if not tokens or not any(t.text == '(' for t in tokens): return None
    words = [t.text for t in tokens]
    if '=' in words and words.index('=') < words.index('('): return None
    if words[0] in ('struct', 'class', 'enum', 'union'): return None
    opening = words.index('(')
    valid = (opening >= 2 and tokens[opening-1].kind == 'identifier' and words[-1] == ')'
             and words.count('(') == 1 and words.count(')') == 1
             and not any(w in words for w in ('template', 'operator', 'auto', '<', '>', '[', ']', '=', ':'))
             and all(t.kind == 'identifier' or t.text in ('*', '&') for t in tokens[:opening-1])
             and all(t.kind == 'identifier' or t.text in (',', '*', '&') for t in tokens[opening+1:-1]))
    if not valid:
        if definition:
            raise SketchError(f'{path}:{tokens[0].line}: unsupported function declarator; move it to a .cpp with explicit declarations')
        return None
    if definition and tokens[0].conditional:
        raise SketchError(f'{path}:{tokens[0].line}: conditional function definition requires an explicit .cpp bridge')
    return tuple(words)


def functions(source, path):
    tokens = tokenize(source, path)
    definitions, declarations = [], set()
    segment = []
    i = 0
    while i < len(tokens):
        token = tokens[i]
        if token.kind == 'directive':
            i += 1; continue
        if token.text == ';':
            signature = function_signature(segment, path, False)
            if signature: declarations.add(signature)
            segment = []; i += 1; continue
        if token.text == '{':
            if segment and segment[0].text in ('namespace', 'extern'):
                raise SketchError(f'{path}:{segment[0].line}: unsupported namespace/linkage block; use a .cpp source')
            signature = function_signature(segment, path, True)
            if signature:
                definitions.append((segment[0].start, token.start, segment[0].line, signature))
            depth = 1; i += 1
            while i < len(tokens) and depth:
                if tokens[i].kind != 'literal':
                    if tokens[i].text == '{': depth += 1
                    elif tokens[i].text == '}': depth -= 1
                i += 1
            if depth: raise SketchError(f'{path}:{token.line}: unterminated scope')
            segment = []; continue
        if token.text == '}': raise SketchError(f'{path}:{token.line}: unmatched scope')
        segment.append(token); i += 1
    return definitions, declarations


def generate(main, output):
    main = main.resolve()
    if not main.is_file() or main.suffix not in ('.ino', '.pde'):
        raise SketchError('main must name an existing .ino or .pde file')
    others = sorted((p for p in main.parent.iterdir() if p.suffix in ('.ino', '.pde') and p != main), key=lambda p: p.name)
    paths = [main, *others]
    sources = [p.read_text() for p in paths]
    parsed = [functions(s, p) for s, p in zip(sources, paths)]
    declarations = set().union(*(p[1] for p in parsed))
    prototypes = []
    seen = set(declarations)
    first = None
    for index, (definitions, _) in enumerate(parsed):
        for start, end, line, signature in definitions:
            if first is None: first = index, start, line
            if signature not in seen:
                prototypes.append(sources[index][start:end].strip() + ';')
                seen.add(signature)
    if first is None: raise SketchError('sketch has no supported free-function definitions')
    arduino_include = re.compile(r'^\s*#\s*include\s*[<"]Arduino\.h[>"]', re.MULTILINE)
    output_text = '// Generated by sdk-arduboy; edit the original Sketch.\n'
    if not any(arduino_include.search(s) for s in sources): output_text += '#include <Arduino.h>\n'
    for index, (path, source) in enumerate(zip(paths, sources)):
        location = json.dumps(str(path), ensure_ascii=False)
        output_text += f'#line 1 {location}\n'
        if index == first[0]:
            output_text += source[:first[1]] + '\n' + '\n'.join(prototypes) + '\n'
            output_text += f'#line {first[2]} {location}\n' + source[first[1]:]
        else: output_text += source
        output_text += '\n'
    output.parent.mkdir(parents=True, exist_ok=True)
    if not output.exists() or output.read_text() != output_text: output.write_text(output_text)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--main', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try: generate(args.main, args.output)
    except (SketchError, OSError) as error: parser.exit(2, f'sdk-arduboy sketch: {error}\n')
if __name__ == '__main__': main()
