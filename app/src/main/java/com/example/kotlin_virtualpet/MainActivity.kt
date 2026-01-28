package com.example.kotlin_virtualpet

import android.app.Dialog
import android.graphics.Color
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.*
import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("kotlin_virtualpet")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val btnSettings = ImageButton(this).apply {
            setImageResource(android.R.drawable.ic_menu_manage)
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

        // --- SECCIÓN COLORES ---
        val colorLabel = TextView(this).apply {
            text = "COLOR DE LA MASCOTA"
            setTextColor(Color.GRAY)
            gravity = Gravity.CENTER
            setPadding(0, 0, 0, 20)
        }
        controlsLayout.addView(colorLabel)

        val row1 = LinearLayout(this).apply { orientation = LinearLayout.HORIZONTAL; gravity = Gravity.CENTER }

        // Botón Blanco
        row1.addView(createColorBtn(Color.WHITE, 1f, 1f, 1f))
        // Botón Rojo
        row1.addView(createColorBtn(Color.RED, 1f, 0f, 0f))
        // Botón Cian (Muy VHS)
        row1.addView(createColorBtn(Color.CYAN, 0f, 1f, 1f))
        // Botón Verde
        row1.addView(createColorBtn(Color.GREEN, 0f, 1f, 0f))

        controlsLayout.addView(row1)

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

    // Función auxiliar para crear botones de colores rápidamente
    private fun createColorBtn(colorInt: Int, r: Float, g: Float, b: Float): View {
        return Button(this).apply {
            setBackgroundColor(colorInt)
            layoutParams = LinearLayout.LayoutParams(150, 150).apply { setMargins(10, 10, 10, 10) }
            setOnClickListener {
                setPetColorNative(r, g, b)
                Toast.makeText(context, "Color actualizado", Toast.LENGTH_SHORT).show()
            }
        }
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