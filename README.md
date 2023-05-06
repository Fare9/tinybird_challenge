# TinyJSON

TinyJSON is a basic tool to analyze stock prices read from a [NDJSON](http://ndjson.org/) file.

## Goal

The capabilities of this library are only a sketch of what's doable with any other analytical framework, so we want to
continue improving it so that it provides fast and correct information from the input data.

We want you to spend some time understanding the code and making an improvement over it. We don't ask for a specific
change in purpose; we want you to decide what should be done and implement it up to a certain point (under the
specified time constraints). Some possible ideas:
  - Performance: (10x, 100x... faster queries).
  - Scalability (10x, 100x, 1000x... more data).
  - Maintainability.
  - API refactors.
  - Features (new APIs, new functions, etc.)
  - Anything that you consider important and doable (partially at least).

After reading this file and going through the code, pick one change and work on it. If you are unsure about it, choose
whatever it's more comfortable for you and helps showcase your abilities. Don't try implementing several modifications:
we don't need it and you shouldn't have time to do it.

## Solution

You final solution should include:
  - Any code changes or additions done to the current project.
  - A [Markdown](https://en.wikipedia.org/wiki/Markdown) file justifying the decisions made during the development and
  what other improvements could be made to the project.

Once you receive this test you have **7 days to provide a solution**. We consider that a week is enough time to work on
a proposal, but if you need more time because of personal matters, please just let us know.

You can provide the solution as a link to a repository (preferable) or as a series of patches.

## Assumptions

  - You should read the whole README.md and have a look at the code before you start.
  - Feel free to ask any questions at any moment, but it's better if you ask before you dive into making changes.
  - The current code is correct, that is, we haven't added tricky hidden bugs on purpose.
  - If you need business information to make a decision, assume whatever you need and write it down.
  - You should spend **at most 3** hours on the exercise. If you go over that, just write down what it's done and
    what's pending.
  - The explanation is as important as the code changes. Please take some of that time to write things down.
  - The code will be built and run under a recent Linux distribution.
  - You can introduce new libraries if you need them.
  - You can use as many resources as you want from the host machine. There is no need to control or limit them manually.

### Evaluation

We'll evaluate **your explanation** based on:
  - **Clarity**: Developers that haven't seen the project before should be able to understand your reasoning.
  - **Decisions**: Explain **why** you took a decision. Even if it's random or exploratory, we want to know.
  - **Other approaches**: We want to know also about the options you discarded and why.
  - **Design**: Schema design, data processing decisions, algorithms, tradeoffs and so on.
  - **Evolution**: How should the project be improved further in your opinion: Next steps, things to consider in the
  future...
  - When in doubt, it's better to be verbose and over-document things.

We'll evaluate **your code changes** based on:
  - Simplicity: Are the changes clear? We want to see modern understandable C++ and standard programming practises.
  - Stability and reproducibility. Are there any bugs? Does it always return the same value for the same input?
  - Design, data structure, processing flow, algorithms, etc. Is it using the right tool for the right use case?
  - Performance and resource usage. You have infinite resources available but that doesn't mean you need to use them.

We will review your changes to verify if you could be a good fit for the position. If we think you are, we will want to
setup some time to talk about the test (code and notes): Why you decided to do X this way? How would you implement it if
we had some extra requirement? What architecture would you use to support a new feature Y? How fast do you think your
proposal Z should be? Could we make it faster?
