import os
import subprocess
import sys

# Set environment variables for the rebase
env = os.environ.copy()
# Use absolute path to editor.bat
env['GIT_SEQUENCE_EDITOR'] = os.path.abspath('editor.bat')

print("Configuring git user...")
subprocess.run(['git', 'config', 'user.name', 'whitecider'], check=True)
subprocess.run(['git', 'config', 'user.email', 'whitecider@gmail.com'], check=True)

print("Starting rebase...")
# Run rebase
# Using --root to cover all commits
# Using --exec to amend each commit
cmd = [
    'git', 'rebase', '--root',
    '--exec', 'git commit --amend --reset-author --no-edit'
]

try:
    result = subprocess.run(cmd, env=env, check=True, capture_output=True, text=True)
    print(result.stdout)
except subprocess.CalledProcessError as e:
    print(f"Error during rebase: {e}")
    print(e.stdout)
    print(e.stderr)
    sys.exit(1)

print("Rebase completed successfully.")
