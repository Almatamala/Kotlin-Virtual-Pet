package com.example.kotlin_virtualpet // Asegúrate de que este paquete coincida con tu proyecto

import android.app.AlertDialog
import android.app.Dialog
import android.graphics.Color
import android.graphics.Typeface
import android.graphics.drawable.ColorDrawable
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AppCompatDelegate
import androidx.core.content.res.ResourcesCompat
import com.google.androidgamesdk.GameActivity
import com.skydoves.colorpickerview.ColorPickerDialog
import com.skydoves.colorpickerview.listeners.ColorEnvelopeListener
import androidx.core.graphics.toColorInt
import androidx.core.graphics.drawable.toDrawable

class MainActivity : GameActivity() {

    // Variable para persistir el color seleccionado (Blanco por defecto)
    private var currentColorInt: Int = Color.WHITE

    companion object {
        init {
            // Verifica que el nombre de la librería coincida con tu CMakeLists.txt
            System.loadLibrary("kotlin_virtualpet")
        }
    }

    // Declaración de funciones nativas
    external fun setVHSEffectNative(enabled: Boolean)
    external fun isVHSEnabledNative(): Boolean
    external fun setPetColorNative(r: Float, g: Float, b: Float)

    override fun onCreate(savedInstanceState: Bundle?) {
        // FORZAR MODO NOCHE SIEMPRE
        AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES)

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
        // Intentar cargar la fuente Bitcount desde app/res/font/bitcount.ttf
        val bitcount = try {
            ResourcesCompat.getFont(this, R.font.bitcountsingle)
        } catch (e: Exception) {
            Typeface.MONOSPACE
        }

        val configDialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)

        val rootLayout = FrameLayout(this).apply {
            setBackgroundColor("#121212".toColorInt())
        }

        val controlsLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setPadding(50, 50, 50, 50)
        }

        // Título del menú
        val titleMenu = TextView(this).apply {
            text = "CONFIGURACIÓN"
            typeface = bitcount
            setTextColor(Color.WHITE)
            textSize = 28f
            setPadding(0, 0, 0, 100)
        }
        controlsLayout.addView(titleMenu)

        // MODO VHS Switch
        val vhsSwitch = Switch(this).apply {
            text = "MODO VHS    "
            typeface = bitcount
            setTextColor(Color.WHITE)
            textSize = 20f
            isChecked = isVHSEnabledNative()
            setPadding(0, 0, 0, 80)
            setOnCheckedChangeListener { _, checked -> setVHSEffectNative(checked) }
        }
        controlsLayout.addView(vhsSwitch)

        // Botón para elegir color
        val btnPickColor = Button(this).apply {
            text = "ELEGIR COLOR"
            typeface = bitcount
            setTextColor(Color.WHITE)
            setBackgroundColor("#1F1F1F".toColorInt())
            setPadding(40, 40, 40, 40)
            setOnClickListener { showProColorPicker(bitcount) }
        }
        controlsLayout.addView(btnPickColor)

        // Botón Volver
        val btnClose = Button(this).apply {
            text = "VOLVER"
            typeface = bitcount
            setTextColor(Color.GRAY)
            setBackgroundColor(Color.TRANSPARENT)
            setOnClickListener { configDialog.dismiss() }
        }
        val closeParams = FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT,
            Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
        ).apply { setMargins(0, 0, 0, 100) }

        rootLayout.addView(controlsLayout)
        rootLayout.addView(btnClose, closeParams)

        configDialog.setContentView(rootLayout)
        configDialog.show()
    }

    private fun showProColorPicker(bitcountFont: Typeface?) {
        val builder = ColorPickerDialog.Builder(this)
            .setTitle("COLOR DE LA MASCOTA")
            .setPreferenceName("MyColorPicker")
            .setPositiveButton("Confirmar", ColorEnvelopeListener { envelope, _ ->
                currentColorInt = envelope.color

                val r = Color.red(currentColorInt) / 255f
                val g = Color.green(currentColorInt) / 255f
                val b = Color.blue(currentColorInt) / 255f

                setPetColorNative(r, g, b)
            })
            .setNegativeButton("Cancelar") { d, _ -> d.dismiss() }
            .attachAlphaSlideBar(false)
            .attachBrightnessSlideBar(true)
            .apply {
                // Personalización del fondo interno del selector
                val colorPickerView = colorPickerView
                colorPickerView.setInitialColor(currentColorInt)
                colorPickerView.setBackgroundColor("#121212".toColorInt())
            }

        val dialog = builder.create()

        // Forzar fondo oscuro en el contenedor del diálogo
        dialog.window?.setBackgroundDrawable("#121212".toColorInt().toDrawable())

        dialog.show()

        // Aplicar fuente Bitcount al título y botones del diálogo
        val titleId = resources.getIdentifier("alertTitle", "id", "android")
        if (titleId > 0) {
            dialog.findViewById<TextView>(titleId)?.apply {
                typeface = bitcountFont
                setTextColor(Color.WHITE)
            }
        }

        dialog.getButton(AlertDialog.BUTTON_POSITIVE).apply {
            setTextColor(Color.CYAN)
            typeface = bitcountFont
        }
        dialog.getButton(AlertDialog.BUTTON_NEGATIVE).apply {
            setTextColor(Color.GRAY)
            typeface = bitcountFont
        }
    }

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