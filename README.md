# EE4065 – Embedded Digital Image Processing  
### Homework 4  

--

## 📄 Project Description

This repository contains our implementation for **EE4065 – Embedded Digital Image Processing, Homework 4**.  
The project focuses on **Embedded Machine Learning** concepts, specifically deploying trained neural networks onto an ARM Cortex-M based microcontroller (STM32) for handwritten digit recognition.

The main objectives are:

1.  **Feature Extraction:** Implementation of a C function to calculate **Hu Moments** (7 invariant moments) from raw image data to reduce dimensionality (from 784 pixels to 7 features).
2.  **Single Neuron Classifier (Q1):** - Train a Single Neuron model in Python to classify digits as "Zero" or "Non-Zero".
    - [cite_start]Export weights and implement the inference engine in C on STM32[cite: 8].
3.  **Multi-Layer Perceptron (MLP) (Q2):** - Train a Neural Network (Input -> Hidden -> Hidden -> Output) in Python to classify digits from 0 to 9.
    - [cite_start]Implement the forward propagation (matrix multiplication + ReLU activation) in C on STM32[cite: 9].
4.  **Integration & Verification:** Verify the embedded inference results against the Python model predictions using the STM32 "Live Expressions" debug tool.

---

## ⚙️ Tools and Environment
- **Microcontroller:** STM32F4 Series (e.g., Nucleo-F446RE)  
- **IDE:** STM32CubeIDE  
- **Language:** C (Inference) and Python (Training)
- **Libraries:** TensorFlow/Keras, OpenCV (Python), Standard Math Library (C)
- [cite_start]**Dataset:** MNIST (Offline Database) [cite: 6]

---

## 👥 Group Members

| No | Name              | Student ID | Role                  |
|----|-------------------|-------------|-----------------------|
| 1  | Mahmut Nedim Göç  | 150721053   | Q1, Q2 and Reporting  |
| 2  | Emre Güner        | 150722031   | Q1, Q2 and Reporting  |

*(Not: Rolleri (Role) iş bölümünüze göre güncelleyebilirsiniz)*

---

## 📅 Due Date
[cite_start]**December 26, 2025 – 23:59 PM** [cite: 3]
