package com.example.kotlin_virtualpet

import android.app.Dialog
import android.graphics.Color
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.*
import com.google.androidgamesdk.GameActivity
import com.skydoves.colorpickerview.ColorPickerDialog
import com.skydoves.colorpickerview.listeners.ColorEnvelopeListener

class MainActivity : GameActivity() {
    private var currentColorInt: Int = Color.WHITE
    companion object {
        init {
            System.loadLibrary("kotlin_virtualpet")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val btnSettings = ImageButton(this).apply {
            setImageResource(R.drawable.settings_icon)
            setBackgroundColor(Color.TRANSPARENT)
        }

        val params = FrameLayout.LayoutParams(180, 180).apply {
            gravity = Gravity.BOTTOM or Gravity.END
            setMargins(0, 0, 50, 50)
        }

        addContentView(btnSettings, params)
        btnSettings.setOnClickListener { showConfigurationScreen() }
    }

    private fun showConfigurationScreen() {
        val configDialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val rootLayout = FrameLayout(this).apply { setBackgroundColor(Color.parseColor("#121212")) }

        val controlsLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
        }

        rootLayout.addView(controlsLayout, FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.CENTER))

        // --- SECCIÓN VHS ---
        val vhsSwitch = Switch(this).apply {
            text = "MODO VHS"
            setTextColor(Color.WHITE)
            textSize = 20f
            isChecked = isVHSEnabledNative()
            setPadding(0, 0, 0, 80)
            setOnCheckedChangeListener { _, checked -> setVHSEffectNative(checked) }
        }
        controlsLayout.addView(vhsSwitch)

        // --- SECCIÓN SELECTOR DE COLOR ---
        val colorLabel = TextView(this).apply {
            text = "PERSONALIZAR COLOR"
            setTextColor(Color.GRAY)
            gravity = Gravity.CENTER
            setPadding(0, 0, 0, 40)
        }
        controlsLayout.addView(colorLabel)

        // Botón para abrir el selector
        val btnPickColor = Button(this).apply {
            text = "ELEGIR COLOR"
            setBackgroundColor(Color.DKGRAY)
            setTextColor(Color.WHITE)
            setOnClickListener {
                // Aquí lanzamos un selector simple basado en un array de predefinidos
                // o podrías integrar un "Color Wheel" si añades la dependencia.
                // Por ahora, usemos un Grid de colores expandido:
                showProColorPicker()
            }
        }
        controlsLayout.addView(btnPickColor)

        // Botón cerrar (X)
        val btnClose = ImageButton(this).apply {
            setImageResource(android.R.drawable.ic_menu_close_clear_cancel)
            setBackgroundColor(Color.TRANSPARENT)
            setColorFilter(Color.WHITE)
        }
        rootLayout.addView(btnClose, FrameLayout.LayoutParams(180, 180).apply {
            gravity = Gravity.BOTTOM or Gravity.END
            setMargins(0, 0, 50, 50)
        })

        btnClose.setOnClickListener { configDialog.dismiss() }
        configDialog.setContentView(rootLayout)
        configDialog.show()
    }

    private fun showProColorPicker() {
        ColorPickerDialog.Builder(this)
            .setTitle("Color de la Mascota")
            // --- AQUÍ ESTÁ EL TRUCO: Establecemos el color inicial ---
            .setPreferenceName("MyColorPicker") // Opcional: lo guarda incluso si cierras la app
            .setPositiveButton("Confirmar", ColorEnvelopeListener { envelope, _ ->
                // 1. Guardamos el color para la próxima vez que abramos el menú
                currentColorInt = envelope.color

                // 2. Convertimos a Float para C++
                val r = Color.red(currentColorInt) / 255f
                val g = Color.green(currentColorInt) / 255f
                val b = Color.blue(currentColorInt) / 255f

                setPetColorNative(r, g, b)
            })
            .setNegativeButton("Cancelar") { dialog, _ -> dialog.dismiss() }
            .attachAlphaSlideBar(false)
            .attachBrightnessSlideBar(true)
            .apply {
                // Sincronizamos el punto del selector con el color guardado
                val colorPickerView = colorPickerView
                colorPickerView.setInitialColor(currentColorInt)
            }
            .show()
    }

    external fun setVHSEffectNative(enabled: Boolean)
    external fun isVHSEnabledNative(): Boolean
    external fun setPetColorNative(r: Float, g: Float, b: Float)

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideSystemUi()
    }

    private fun hideSystemUi() {
        window.decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN)
    }
}