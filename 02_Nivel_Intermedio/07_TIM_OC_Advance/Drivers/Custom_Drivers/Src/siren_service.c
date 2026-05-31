/**
 * @file siren_service.c
 * @author Mamani Flores Carlos (UTN FRT)
 * @brief Implementación de la modulación asíncrona de perfiles de audio y luces baliza.
 * @details Desarrolla las cinemáticas modulares no bloqueantes para la generación
 * de perfiles lineales e inerciales de sirenas (*Wail*, *Yelp*, *Hi-Lo*, *War Siren*),
 * coordinando en tiempo real las ráfagas estroboscópicas de las balizas mediante la PAL.
 * @version 1.0
 * @date 2026
 */

#include "siren_service.h"
#include <stddef.h>

/**
 * @brief Inicializa el descriptor de servicio de la sirena y balizas coordinadas.
 * @param siren Puntero a la estructura del servicio de sirena (objeto).
 * @param tg Puntero al objeto instanciado del generador de tonos (Capa 2).
 * @param led1 Descriptor del canal PWM para el bloque estroboscópico izquierdo (Rojo).
 * @param led2 Descriptor del canal PWM para el bloque estroboscópico derecho (Azul).
 * @param pal_io Instancia de la PAL conteniendo las abstracciones del sistema operacional.
 */
void SIREN_SERVICE_Init(siren_service_t *siren, tone_gen_t *tg, generic_pwm_t led1, generic_pwm_t led2, hal_interface_t pal_io) {
	if (siren == NULL || tg == NULL || pal_io.get_tick == NULL) return;

	siren->tone_gen = tg;
	siren->led_ch1 = led1;
	siren->led_ch2 = led2;
	siren->pal = pal_io;
	siren->current_mode = MODE_OFF;
	siren->last_siren_tick = 0;
	siren->last_led_tick = 0;
	siren->current_freq = 600;
	siren->freq_step = 10;
}

/**
 * @brief Conmuta el modo operativo de la sirena cargando perfiles dinámicos atómicos.
 * @details Reinicia inmediatamente las bases de tiempo relativas para evitar retardos
 * de fase en la conmutación de modos mecánicos. Configura las condiciones de contorno
 * de frecuencia y tasas de cambio (`freq_step`) específicas de cada cinemática de audio.
 * @param siren Puntero a la estructura del servicio de sirena.
 * @param new_mode Identificador del nuevo modo seleccionado (SirenMode_t).
 */
void SIREN_SERVICE_SetMode(siren_service_t *siren, SirenMode_t new_mode) {
	if (siren == NULL || siren->pal.get_tick == NULL) return;

	siren->current_mode = new_mode;

	/* Reinicio inmediato de bases de tiempo para sincronizar los quiebres de los osciladores */
	siren->last_siren_tick = siren->pal.get_tick();
	siren->last_led_tick = siren->pal.get_tick();

	if (new_mode == MODE_OFF) {
		TONE_GENERATOR_Stop(siren->tone_gen);
		if (siren->pal.pwm_write != NULL) {
			siren->pal.pwm_write(siren->led_ch1, 0);
			siren->pal.pwm_write(siren->led_ch2, 0);
		}
	} else {
		/* Configuración e inicialización atómica de perfiles cinemáticos */
		switch (new_mode) {
			case MODE_WAIL:
				siren->current_freq = 600;
				siren->freq_step = 12; // Barrido lineal suave
				break;

			case MODE_YELP:
				siren->current_freq = 700;
				siren->freq_step = 40; // Barrido lineal rápido y agresivo
				break;

			case MODE_HI_LO:
				siren->current_freq = 700; // Arranca en el tono bajo europeo
				siren->freq_step = 0;      // Alternancia discreta manejada por tiempo en el Update
				break;

			case MODE_WAR:
				siren->current_freq = 300; // Tono muy grave de motor apagado
				siren->freq_step = 6;      // Inercia de aceleración pesada
				break;

			case MODE_HORN:
				siren->current_freq = 420; // Bocina ronca constante
				siren->freq_step = 0;      // Frecuencia fija sin rampa
				break;

			default:
				break;
		}
	}
}

/**
 * @brief Actualiza de forma asíncrona la máquina de estados de audio y señalización visual.
 * @details Esta función debe ciclar libremente en el loop principal (`while(1)`).
 * Se subdivide en dos secciones desacopladas mediante temporización no bloqueante basada en Systick:
 * 1) Modulación cinemática del audio en pasos fijos de 20 ms.
 * 2) Modulación estroboscópica adaptativa de las balizas según la agresividad del modo operativo.
 * @param siren Puntero a la estructura del servicio de sirena.
 */
void SIREN_SERVICE_Update(siren_service_t *siren) {
	if (siren == NULL || siren->current_mode == MODE_OFF || siren->pal.get_tick == NULL) return;

	uint32_t now = siren->pal.get_tick();

	/* ========================================================================== */
	/* 1. MÁQUINA DE MODULACIÓN DE AUDIO (Barridos de Frecuencia No Bloqueantes)  */
	/* ========================================================================== */
	if (now - siren->last_siren_tick >= 20) {
		siren->last_siren_tick = now;

		switch (siren->current_mode) {
		case MODE_WAIL: /* Rampa lineal lenta (Ambulancia) */
			siren->current_freq += siren->freq_step;
			if (siren->current_freq >= 1400 || siren->current_freq <= 600) {
				siren->freq_step *= -1; /* Inversión de signo de la rampa */
			}
			TONE_GENERATOR_SetFrequency(siren->tone_gen, siren->current_freq);
			break;

		case MODE_YELP: /* Rampa lineal de alta tasa de cambio (Patrulla) */
			siren->current_freq += siren->freq_step;
			if (siren->current_freq >= 1600 || siren->current_freq <= 700) {
				siren->freq_step *= -1;
			}
			TONE_GENERATOR_SetFrequency(siren->tone_gen, siren->current_freq);
			break;

		case MODE_HI_LO: /* Oscilador bitonal discreto optimizado */
			{
				// Detectamos de forma dinámica el tono correspondiente al semiciclo de 500ms
				uint32_t nuevo_tono = ((now / 500) % 2 == 0) ? 700 : 1100;

				// SOLO actualizamos el generador si hubo un cambio de tono efectivo (Flanco)
				if (siren->current_freq != nuevo_tono) {
					siren->current_freq = nuevo_tono;
					TONE_GENERATOR_SetFrequency(siren->tone_gen, siren->current_freq);
				}
			}
			break;

		case MODE_WAR: /* Sirena de Ataque Aéreo (Inercia pesada) */
			siren->current_freq += siren->freq_step;

			// Perfil cinemático: Subida rápida, caída pesada y lenta
			if (siren->current_freq >= 900) {
				siren->freq_step = -4;  // Desaceleración lenta por fricción
			} else if (siren->current_freq <= 300) {
				siren->freq_step = 6;   // Aceleración nominal por motor
			}

			TONE_GENERATOR_SetFrequency(siren->tone_gen, siren->current_freq);
			break;

		case MODE_HORN: /* Bocina de aire estática optimizada */
			// Al ser estática, solo se setea una vez en el SetMode.
			// Evitamos re-escribir la variable en memoria cada 20ms de forma cíclica.
			break;

		default: break;
		}
	}

	/* ========================================================================== */
	/* 2. MÁQUINA ESTROBOSCÓPICA (Control Asíncrono de Balizas LED escalonado)     */
	/* ========================================================================== */
	uint32_t led_interval = 150; // Velocidad por defecto (Rápida para Yelp e Hi-Lo)

	if (siren->current_mode == MODE_WAIL) {
		led_interval = 400; // Parpadeo lento y espaciado
	} else if (siren->current_mode == MODE_WAR) {
		led_interval = 600; // Destello pesado de advertencia civil para bombardeo
	} else if (siren->current_mode == MODE_HORN) {
		led_interval = 80;  // Ráfagas estroboscópicas ultra rápidas acompañando el bocinazo
	}

	if (now - siren->last_led_tick >= led_interval) {
		siren->last_led_tick = now;

		static uint8_t toggle = 0;
		toggle = !toggle;

		if (siren->pal.pwm_write != NULL) {
			if (toggle) {
				siren->pal.pwm_write(siren->led_ch1, 800);
				siren->pal.pwm_write(siren->led_ch2, 0);
			} else {
				siren->pal.pwm_write(siren->led_ch1, 0);
				siren->pal.pwm_write(siren->led_ch2, 800);
			}
		}
	}
}
