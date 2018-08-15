// Using a better Graphical API.
#pragma once

#include <windows.h>

namespace SnippingToolUIForm {

  using namespace System;
  using namespace System::ComponentModel;
  using namespace System::Collections;
  using namespace System::Windows::Forms;
  using namespace System::Data;
  using namespace System::Drawing;

  /// <summary>
  /// Summary for MyForm
  /// </summary>
  public ref class MyForm : public System::Windows::Forms::Form {
  public:
    MyForm(void) {
      // Honkwan: Screenshot now.
      bmp = gcnew Bitmap(1920, 1080);
      gr = Graphics::FromImage(bmp);
      gr->CopyFromScreen(0, 0, 0, 0, bmp->Size);
      InitializeComponent();
    }

  protected:
    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    ~MyForm() {
      if (components) {
        delete components;
      }
    }
  private: System::ComponentModel::IContainer^  components;
  protected:

  private:
    /// <summary>
    /// Required designer variable.
    /// </summary>


    // Honkwan added.
    Bitmap^ bmp;
    Graphics^ gr;
    Point start_location;
    Point current_location;
    bool mouse_down = false;

#pragma region Windows Form Designer generated code
    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    void InitializeComponent(void) {
      this->SuspendLayout();
      // 
      // MyForm
      // 
      this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
      this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
      //this->ClientSize = System::Drawing::Size(1484, 261);
      this->DoubleBuffered = true;
      this->Name = L"MyForm";
      this->Text = L"MyForm";
      this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
      this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::RePaint);
      this->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::StartCapture);
      this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::RefreshRectangle);
      this->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::EndCapture);
      this->ResumeLayout(false);
      // Fullscreen + boarderless.
      CenterToScreen();
      FormBorderStyle = Windows::Forms::FormBorderStyle::None;
      WindowState = FormWindowState::Maximized;
      // Change the backgroud image.
      this->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(bmp));;

    }
#pragma endregion
  private: System::Void MyForm_Load(System::Object^  sender, System::EventArgs^  e) {}


  private: System::Void StartCapture(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
    // Start the snip on mouse down
    start_location = e->Location;
    mouse_down = true;

  }
  private: System::Void EndCapture(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
    mouse_down = false;
  }
  private: System::Void RefreshRectangle(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
    current_location = e->Location;
    Invalidate();
  }

  private: System::Void RePaint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
    Brush^ br_white = gcnew SolidBrush(Color::FromArgb(123, Color::White));
    if (!mouse_down)
      return;

    System::Drawing::Rectangle rectangle;
    rectangle.X = min(current_location.X, start_location.X);
    rectangle.Y = min(current_location.Y, start_location.Y);
    rectangle.Width = max(current_location.X, start_location.X) - rectangle.X;
    rectangle.Height = max(current_location.Y, start_location.Y) - rectangle.Y;

    e->Graphics->DrawRectangle(gcnew Pen(Color::Red, 1.0f), rectangle);
    e->Graphics->FillRectangle(br_white, 0, 0, 1920, rectangle.Y);
    e->Graphics->FillRectangle(br_white, 0, rectangle.Y + rectangle.Height, 1920, 1080);
    e->Graphics->FillRectangle(br_white, 0, rectangle.Y, rectangle.X, rectangle.Height);
    e->Graphics->FillRectangle(br_white, rectangle.X + rectangle.Width, rectangle.Y, 1920, rectangle.Height);
  }

  };
}
