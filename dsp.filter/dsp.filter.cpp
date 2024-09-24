#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
using namespace std;
#define M_PI 3.141592653589793
struct ADCFILEHEADER
{
	double dt; //!< Период дискретизации в секундах
	unsigned long h; //!< Разрядность АЦП
	unsigned long fN; //!< Общее количество отсчетов в файле
	float Am; //!< Амплитуда, соответствующая максимальному значению целочисленного отсчета
	unsigned long flags; //!< Флаги
	char reserv[8]; //!< Резерв
};
typedef struct ADCFILEHEADER* LPADCFILEHEADER; //!< Указатель на ADCFILEHEADER
#define ADCF_DATA_INTEGER 0x00000000 //!< Тип данных - целочисленные
#define ADCF_DATA_FLOAT 0x00000001 //!< Тип данных - float
#define ADCF_TYPE_REAL 0x00000000 //!< Тип отсчетов - действительные
#define ADCF_TYPE_COMPLEX 0x00000002 //!< Тип отсчетов – комплексные

const int N = 256; //Длина фильтра
float Fd = 48000; //Частота дискретизации входных данных
//float Fs = 2000; //Частота среза идеального ФНЧ
float Fs = 3000; //Частота полосы пропускания
float Fx = 3900; //Частота полосы затухания

bool Write_adc_in(vector<float>& in_array_for_file)
{
	ADCFILEHEADER head;
	head.Am = 1.0;
	head.h = 32;
	head.dt = 1.0 / Fd;
	head.fN = in_array_for_file.size();
	head.flags = ADCF_DATA_FLOAT;
	const char* FName = "D:\\file\\cic_in.adc"; //Путь к файлу
	float im = 0.0;
	/*РАБОТА С БИНАРНЫМ ФАЙЛОМ*/
	ofstream out(FName, ios::binary); //Ставим режим "бинарный файл"
	out.write((char*)&head, sizeof(head)); //Записываем в файл структуру заголовка
	for (int i = 0; i < in_array_for_file.size(); ++i)
	{
		out.write((char*)&in_array_for_file[i], sizeof(in_array_for_file[i])); // Записываем действительные отсчеты из вектора
	}
	out.close();
	return true;
}

bool Write_adc(vector<float> array)
{
	ADCFILEHEADER head;
	head.Am = 1.0;
	head.h = 32;
	head.dt = 1.0 / Fd;
	head.fN = array.size();
	head.flags = ADCF_DATA_FLOAT;
	const char* FName = "D:\\file\\cic_out.adc"; //Путь к файлу
	float im = 0.0;
	//РАБОТА С БИНАРНЫМ ФАЙЛОМ
	ofstream out(FName, ios::binary); //Ставим режим "бинарный файл"
	out.write((char*)&head, sizeof(head)); //Записываем в файл структуру заголовка
	for (int i = 0; i < array.size(); ++i)
	{
		out.write((char*)&array[i], sizeof(array[i])); // Записываем действительные отсчеты из вектора
	}
	out.close();
	return true;
}

void Write_to_file(float* pout, int sizeIn)
{
	vector<float> array_for_file;
	array_for_file.resize(sizeIn);
	for (int i = 0; i < array_for_file.size(); i++)
	{
		array_for_file[i] = pout[i];
	}
	ofstream write;
	write.open("D:\\file\\cic.txt", ios::trunc | ios::in | ios::out);
	for (int i = 0; i < sizeIn; i++)
	{
		write << i << ". " << array_for_file[i] << ";\n";
	}
	write.close();
	Write_adc(array_for_file);
}

void Generate_digital_signal_meandr(float* pin, int sizeIn)
{
	/*for (int i = 0; i < sizeIn; i++)
	{
		if (i % 2 != 0)
		{
			pin[i] = 0;
		}
		else
		{
			pin[i] = 2;
		}
	}*/
	for (int i = 0; i < sizeIn; i++)
	{
		pin[i] = 10 * sin(float(2 * M_PI * 2e3 * i / Fd)) + 1.5 * sin(float(2 * M_PI * 1.15423e3 * i / Fd)) + 7 * cos(float(2 * M_PI * 1.24e3 * i / Fd)) + 2.8 * sin(float(2 * M_PI * 7e4 * i / Fd));
	}
	cout << "Заполненный массив\n";
	for (int i = 0; i < sizeIn; i++)
	{
		cout << i << ".\t" << pin[i] << "\n";
	}
	vector<float> in_array_for_file;
	in_array_for_file.resize(sizeIn);
	for (int i = 0; i < in_array_for_file.size(); i++)
	{
		in_array_for_file[i] = pin[i];
	}
	Write_adc_in(in_array_for_file);
}

void Generate_digital_signal_sin(float* pin, int sizeIn)
{
	for (int i = 0; i < sizeIn; i++)
	{

		pin[i] = 10 * sin(2 * M_PI * 10000 * i / Fd);
	}
	cout << "Заполненный массив\n";
	for (int i = 0; i < sizeIn; i++)
	{
		cout << i << ".\t" << pin[i] << "\n";
	}
	vector<float> in_array_for_file;
	in_array_for_file.resize(sizeIn);
	for (int i = 0; i < in_array_for_file.size(); i++)
	{
		in_array_for_file[i] = pin[i];
	}
	Write_adc_in(in_array_for_file);
}

void Filter(float* pin, int sizeIn)
{
	float H[N] = { 0 }; //Импульсная характеристика фильтра
	float H_id[N] = { 0 }; //Идеальная импульсная характеристика
	float W[N] = { 0 }; //Весовая функция
	float out[256];
	//double Fc = 0.;

	//Расчет импульсной характеристики фильтра
	float Fc = (Fs + Fx) / (2 * Fd);

	//Fc = Fs / Fd;

	for (int i = 0; i < N; i++)
	{
		if (i == 0) H_id[i] = 2 * M_PI * Fc;
		else H_id[i] = (2 * M_PI * Fc * sinf(2 * M_PI * Fc * i)) / (2 * M_PI * Fc * i);
		//W[i] = 0.42 - 0.5 * cosf((2 * M_PI * i) / (N - 1)) + 0.08 * cosf((4 * M_PI * i) / (N - 1));// окно Блэкмана
		W[i] = 0.35875 - 0.48829 * cos(float((2 * M_PI * i) / (N - 1)) + 0.14128 * cosf((4 * M_PI * i) / (N - 1))) - 0.001168 * cosf((2 * M_PI * i) / (N - 1));// окно Блэкмана-Харриса
		//W[i] = 0.54 - 0.46 * cosl((2 * M_PI * i) / (N - 1));// окно Хемминга
		//W[i] = cos(float((M_PI * i) / (N - 1)));// косинус окно
		H[i] = H_id[i] * W[i];
	}

	//Нормировка импульсной характеристики
	double SUM = 0;
	for (int i = 0; i < N; i++) SUM += H[i];
	for (int i = 0; i < N; i++) H[i] /= SUM;//сумма коэффициентов равна 1 

	//Фильтрация входных данных
	for (int i = 0; i < sizeIn; i++)
	{
		out[i] = 0.;
		for (int j = 0; j < N - 1; j++) // Свертка импульсной характеристики фильтра со входной последовательностью
		{
			if (i - j >= 0)
				out[i] += H[j] * pin[i - j];
		}
	}
	for (int i = 0; i < sizeIn; i++)
	{
		cout << i << ".\t" << out[i] << "\n";
	}
	Write_to_file(out, sizeIn);
}

int main()
{
	system("chcp 1251");
	const int SIZE = 256;
	float in[SIZE]{};
	Generate_digital_signal_sin(in, SIZE);
	//Generate_digital_signal_meandr(in, SIZE);
	cout << "---------------------------------\n";
	Filter(in, SIZE);
	return 0;
}