# Yukon Solitaire Report Template

This file is the report skeleton so we do not leave the whole write-up to the last week.

We can copy this into the final report document later, but for now it is the structure we follow while building.

## 1. Cover Page

Things to place here:
- course name and course number
- project title
- group member names
- student numbers if required by the course
- one photo of each group member if the report instructions still require that
- submission date

Suggested title for now:

`Yukon Solitaire - Terminal and GUI Implementation with a Shared C Backend`

## 2. Contribution Statements

This section should be personal and honest.

The point is not to sound fancy. The point is to show who worked on what.

Suggested mini-structure per person:
- what parts of the project they worked on
- what they implemented directly
- what testing / debugging / report writing they handled
- where the work was shared with the teammate

## 3. Requirements

Things to describe here:
- what the assignment required
- the difference between STARTUP phase and PLAY phase
- mandatory terminal version
- mandatory GUI version
- shared backend idea
- linked-list requirement for deck, tableau, and foundations

## 4. Analysis

Good things to explain here:
- why Yukon is slightly different from simpler solitaire variants
- why linked lists fit this project pretty well
- what parts of the problem are state-heavy
- where input validation matters

## 5. Design

Things worth describing:
- main data structures like `Card`, `Deck`, `TableauColumn`, `FoundationPile`, and `GameState`
- module split between `src/core`, `src/cli`, and `src/gui`
- why command parsing is separate from command execution
- how the GUI reuses the backend instead of reimplementing game logic

## 6. Implementation

This is where we can talk through the actual code in a practical way.

Possible subsections:
- card parsing and formatting
- linked-list helpers
- deck load/save
- shuffling
- game setup and dealing
- move validation and execution
- terminal rendering
- GUI bridge and Tkinter frontend

## 7. Testing

Things to include:
- manual test checklist
- legal and illegal move tests
- malformed file tests
- compile checks with warnings enabled
- memory / sanitizer checks

This section should feel concrete.

It is better to mention actual tested flows than to write vague lines like "the program was tested thoroughly".

## 8. Generative AI Use

This section should be short and direct.

Suggested things to mention:
- AI was used to help structure work, explain code ideas, and draft some implementation support
- the final code and report were still reviewed, tested, and adjusted by the group
- responsibility for correctness stayed with the students

## 9. Discussion

Possible points:
- what design choices worked well
- what was harder than expected
- what could be improved if there were more time
- tradeoffs between speed of development and cleaner architecture

## 10. Conclusion

Short recap:
- what got implemented
- whether the main assignment goals were achieved
- what the final result supports in terminal and GUI form

## 11. Appendix

Possible appendix material:
- command examples
- tricky test cases
- screenshots
- extra diagrams

## Writing note

When we write the final report, we should keep the tone clear and technical, but not try to sound like a research paper if that is not what the course wants.
