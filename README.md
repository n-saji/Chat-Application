# CSE4/589: Modern Network Concepts - Programming Assignment 1

Welcome to Programming Assignment 1 for CSE4/589: Modern Network Concepts, focused on developing a text chat application. This assignment involves building both client and server components to enable text communication over TCP connections. Follow the guidelines below to start your project.

## Objectives

The main goal is to develop a text chat application comprising one chat server and multiple chat clients, facilitating communication over TCP connections. The project has to be done in one stage.

## Getting Started

You have the option to complete this assignment in either C, C++ or Python. To select your language of choice, execute the following command in your terminal. This will merge the chosen language branch into the main branch of your repository.

For C:

```
cd <path_to_your_local_repo>
git fetch --all
git merge origin/c -m "lang option: c" --no-ff && git push origin main
```

For C++:

```
cd <path_to_your_local_repo>
git fetch --all
git merge origin/cpp -m "lang option: cpp" --no-ff && git push origin main
```

For Python:

```
cd <path_to_your_local_repo>
git fetch --all
git merge origin/python -m "lang option: python" --no-ff && git push origin main
```

## Directory Structure

After merging the chosen branch, your directory should follow the structure outlined below:

### For C:

```
.
├── README.md
├── assignment1_package.sh
├── grader
│   ├── grader.cfg
│   └── grader_controller
└── pa1
    ├── Makefile
    ├── include
    │   ├── global.h
    │   └── logger.h
    ├── logs
    ├── object
    └── src
        ├── assignment1.c
        └── logger.c
```

### For C++:

```
.
├── README.md
├── assignment1_package.sh
├── grader
│   ├── grader.cfg
│   └── grader_controller
└── pa1
    ├── Makefile
    ├── include
    │   ├── global.h
    │   └── logger.h
    ├── logs
    ├── object
    └── src
        ├── assignment1.cpp
        └── logger.cpp
```

### For Python:

```
.
├── README.md
├── assignment1_package_py.sh
├── grader
│   ├── grader.cfg
│   └── grader_controller
└── pa1
    └── src
        ├── assignment1.py
        └── logger.py
```

## Resources

To assist you with the assignment, please refer to the following resources:

- **Assignment Handout:** [Link](https://docs.google.com/document/d/184DnVFBqZ1siLA_H5KsFBbI3oOa0Trhv0JPWN97Wjm0/edit?usp=sharing)
- **Assignment Template:** [Link](https://docs.google.com/document/d/1GibGutfnvbqd0Lvt9HXhZ6sHC3oWjnwjVIIfkD6iy7o/edit?usp=sharing)
- **Assignment Report Template:** [Link](https://docs.google.com/document/d/16XV-6gFnkXeXoumIAj4kjlw10dBsxqKe-A-3N9v0MFY/edit?usp=sharing)

Please ensure you read through the handout thoroughly before starting the assignment to understand the requirements and deliverables.

## Support

For any doubts or clarifications, please refer to the [Piazza forum](https://piazza.com/class/m05fx9f67l5390) for our course. Make sure to follow the forum for updates and discussions related to the assignment.

## Submission Guidelines

- Use the `assignment1_package_py.sh` script to package your submission.
- Ensure your code compiles and runs as expected in the provided environment.
- Submit your assignment through the designated submission portal before the deadline.

Wishing you the best with your assignment!
