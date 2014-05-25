/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */

#ifndef MIXER_HPP_
#define MIXER_HPP_

#include <cstdlib>

struct mixer_input {
  void* data;       // Wskaźnik na dane w FIFO
  size_t len;       // Liczba dostępnych bajtów
  size_t consumed;  // Wartość ustawiana przez mikser, wskazująca, ile bajtów należy
                    // usunąć z FIFO.
};

void mixer(
  struct mixer_input* inputs, size_t n,  // tablica struktur mixer_input, po jednej strukturze na każdą
                                         // kolejkę FIFO w stanie ACTIVE
  void* output_buf,                      // bufor, w którym mikser powinien umieścić dane do wysłania
  size_t* output_size,                   // początkowo rozmiar output_buf, następnie mikser umieszcza
                                         // w tej zmiennej liczbę bajtów zapisanych w output_buf
  unsigned long tx_interval_ms           // wartość zmiennej TX_INTERVAL
);

#endif /* MIXER_HPP_ */
