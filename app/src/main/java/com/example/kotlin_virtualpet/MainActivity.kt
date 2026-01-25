package com.example.kotlin_virtualpet

import android.app.Dialog
import android.graphics.Color
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ImageButton
import android.widget.LinearLayout
import android.widget.Switch
import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("kotlin_virtualpet")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Botón de tuerca
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
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.CENTER))

        // Switch VHS
        val vhsSwitch = Switch(this).apply {
            text = "Efecto VHS"
            setTextColor(Color.WHITE)
            isChecked = isVHSEnabledNative()
            setOnCheckedChangeListener { _, checked -> setVHSEffectNative(checked) }
        }
        controlsLayout.addView(vhsSwitch)

        // Botones de color
        val btnRed = Button(this).apply {
            text = "Rojo"
            setOnClickListener { setPetColorNative(1.0f, 0.0f, 0.0f) }
        }
        controlsLayout.addView(btnRed)

        val btnWhite = Button(this).apply {
            text = "Blanco"
            setOnClickListener { setPetColorNative(1.0f, 1.0f, 1.0f) }
        }
        controlsLayout.addView(btnWhite)

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