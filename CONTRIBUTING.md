# Contributing to Mustard

Thank you for your interest in contributing to Mustard — your help makes this project better for everyone and contributes to exploring cutting-edge science and technologies.

This document explains how to report issues, propose changes, and prepare contributions so they can be reviewed and merged quickly.

- [Contributing to Mustard](#contributing-to-mustard)
  - [Reporting issues](#reporting-issues)
  - [Proposing changes (pull requests)](#proposing-changes-pull-requests)
  - [Code style and tests](#code-style-and-tests)
  - [Branching and commit messages](#branching-and-commit-messages)
  - [Pull request checklist](#pull-request-checklist)
  - [Reviewing and maintaining](#reviewing-and-maintaining)
  - [Communication and Code of Conduct](#communication-and-code-of-conduct)
  - [Getting help](#getting-help)
  - [Maintainer](#maintainer)

## Reporting issues
- Search existing issues before opening a new one to avoid duplicates.
- When opening an issue include:
  - A clear title and description of the problem or feature request.
  - Environment information (OS, compiler version, any relevant dependency versions).
  - Steps to reproduce (minimum reproducible example if possible).
  - Expected vs actual behavior.
  - Logs or stack traces, and any input files needed to reproduce the issue.
  - If relevant, attach small test files or a short script that reproduces the problem.

## Proposing changes (pull requests)
- Fork the repository and create a branch from `main` with a descriptive name, e.g. `feature/add-xyz`, `fix/issue-123` or `docs/update-abc`.
- Make small, focused commits with clear messages.
- Rebase or merge the latest `main` into your branch to keep history clean and resolve conflicts locally.
- Open a pull request targeting `main`. In the PR description, include:
  - A short summary of the change and why it is needed.
  - Related issue number(s) (e.g. "Fixes #123").
  - Any design decisions or trade-offs.
  - How you tested the change and instructions to reproduce the test.

## Code style and tests
- Follow the project's [coding style guide](STYLE_GUIDE.md). If the repository contains a linter, run it before submitting.
- Add unit or integration tests for new features and bug fixes where applicable.
- If the project uses a code formatter (black, clang-format, etc.), run it before committing.
- Ensure all tests pass locally.

## Branching and commit messages
- Use short, descriptive branch names: `feature/`, `fix/`, `chore/`, `docs/` prefixes help reviewers.
- Commit message format:
  - One-line summary (50 characters or less).
  - Blank line.
  - More detailed description, if necessary (wrap at ~72 characters).
- Use present tense and imperative mood, with the first letter of the sentence capitalized, e.g. "Add command-line option for X".

## Pull request checklist
Before requesting review, ensure:
- [ ] I have read the [contributing guidelines](CONTRIBUTING.md)
- [ ] The PR targets the `main` branch.
- [ ] I linked related issues and provided context in the PR description.
- [ ] My code follows the [coding style guide](STYLE_GUIDE.md) and linting rules.
- [ ] I added/updated unit tests where applicable.
- [ ] I updated relevant documentation (README, Doxygen, or design docs).
- [ ] I ran the test-suite locally and all tests pass.
- [ ] All CI checks pass.

## Reviewing and maintaining
- Be responsive to review comments: update your branch, push changes, and add replies explaining choices.
- Keep PRs focused and small where possible — they review faster.
- Maintainers may ask to squash or rework commits before merging. Maintain a cooperative attitude.

## Communication and Code of Conduct
- Be respectful and courteous in issues and PRs. Constructive feedback helps everyone.
- If you are unsure about something, ask maintainers or contributors before implementing a large change.

## Getting help
- If you need assistance getting started, open an issue prefixed with `Help:` and maintainers or developers will try to help.

## Maintainer
[**@zhao-shihan**](https://github.com/zhao-shihan)

Thank you for helping improve Mustard!
