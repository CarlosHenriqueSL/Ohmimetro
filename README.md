# Trabalho 02: Ohmímetro com a BitDogLab

---

## Aluno: 
- **Carlos Henrique Silva Lopes**

## **Link do vídeo de Ensaio**
https://drive.google.com/file/d/1C8X790nI45M2SbMf1tatGsG4godaW3_v/view?usp=sharing

## Objetivos
O projeto pretende mostrar, pelo Display SSD1306 valores de resistências e suas faixas. Ele utiliza a fórmula do divisor de tensão para encontrar o valor da resistência, então, o sistema compara o valor encontrado com os valores comercializados pela E24, e então mostra esse valor na tela do display junto ao valor real e às faixas de cores.

### Principais Arquivos
- **`Ohmimetro.c:`** Contém a lógica principal para fazer a leitura ddo adc, e as operações necessárias para achar o valor mais próximo do resistor na E24 e achar as faixas de cores correspondentes.
- **`lib/:`** Contém os arquivos com caracteres em hexadecimal e arquivos para escrever no ssd1306.
- **`README.md:`** Documentação detalhada do projeto.
