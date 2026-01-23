import re

file_path = '/Users/zhengzihang/my-src/j2me-vm/src/core/Interpreter.cpp'

with open(file_path, 'r') as f:
    content = f.read()

# Regex explanation:
# (\})             : Capture the first closing brace (Group 1)
# \s*              : Match whitespace (including newlines)
# (                : Start Group 2 (intervening content)
#   (?:            : Non-capturing group for comments
#     (?://.*)     : Match // comment until end of line
#     |            : OR
#     (?:/\*[\s\S]*?\*/) : Match /* ... */ comment
#   )
#   \s*            : Whitespace after comment
# )*               : Repeat Group 2 zero or more times
# \}               : The SECOND closing brace (the one we want to remove)
# \s*while\(0\);   : The while(0); part

# Note: We need to capture the whitespace BEFORE the comments too?
# My regex above: (\})\s* ...
# The \s* is NOT captured. So if I replace with $1$2, I lose that whitespace.
# Better: Capture EVERYTHING between the two braces.

# (\})([\s\S]*?)\}\s*while\(0\);
# But I want to ensure the content is only whitespace/comments.
# If I match any content, I might match valid code if I'm not careful.
# e.g. } some_code(); } while(0); -> I should NOT remove the brace here!

# So strict matching of comments/whitespace is important.

pattern = re.compile(r'(\})(\s*(?:(?:(?://.*)|(?:/\*[\s\S]*?\*/))\s*)*)\}\s*while\(0\);', re.MULTILINE)

def replacer(match):
    # match.group(1) is the first '}'
    # match.group(2) is the whitespace and comments between the braces
    return match.group(1) + match.group(2) + 'while(0);'

new_content = pattern.sub(replacer, content)

with open(file_path, 'w') as f:
    f.write(new_content)

print("Fixed Interpreter.cpp (round 2)")
