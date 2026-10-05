<!-- local-agent checkpoint instructions -->
## Local Agent task checkpoint

The checkpoint path below is relative to the directory containing this
instruction file.

At the start of a task and after context compaction, read
`.zcode/local-agent-checkpoint.md` if it exists. Treat it as working notes;
confirm claims against the files and the latest user instructions.

After substantial progress, before returning control, and before a large
context-consuming operation, update that file with a concise checkpoint:

- Current goal and the user's constraints.
- Files changed and important decisions, with paths to durable artifacts.
- Commands and checks actually run, their results, and known failures.
- Remaining tasks and the next concrete step.

Keep the checkpoint short and current. Preserve useful existing notes. Do not
store credentials or claim checks passed without evidence. A new session should
be able to continue from the checkpoint and the files on disk.
<!-- end local-agent checkpoint instructions -->
