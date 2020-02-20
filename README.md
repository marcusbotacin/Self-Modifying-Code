# Self-Modifying-Code

Additional Material for the Self Modifying Code (SMC) paper.

## Authors

Msc. [Marcus Botacin](www.inf.ufpr.br/mfbotacin), under supervision of [Prof. Dr. Marco Zanata](http://web.inf.ufpr.br/mazalves/) and [Prof. Dr. André Grégio](https://sites.google.com/site/argregio/) -- [Department of Informatics](http://web.inf.ufpr.br/dinf/) - [Federal University of Paraná](http://www.ufpr.br/portalufpr/).

## Goal

Provide a security look over architectural implications of SMC execution.

## Repository Organization

* **LinuxSimulation/**: A simple Linux kernel module to evaluate the cost of performing I/O.
* **Cache.Simulator**: A PIN-based, flush-aware cache simulator.
* **Detector**: A PEBS-based SMC detector.
* **Examples**: SMC code used for evaluation.

## Paper

* The article *The Self Modifying Code (SMC)-Aware Processor (SAP): A security look on architectural impact and support* was published in the *Journal of Computer Virology and Hacker Techniques*. [Check Preprint Here](https://github.com/marcusbotacin/Self-Modifying-Code/blob/master/Paper/paper.pdf)

## Challenge

* I included in the *Challenge* directory a SMC-based CTF challenge presented to me by Katharina Bogad (thanks). I think that playing with it is an interesting exercise to understand SMC better, so I decided to include it in this repository.
