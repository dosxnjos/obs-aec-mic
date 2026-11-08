# obs-aec-mic — microfone com cancelamento de eco para o OBS no Windows

[English](README.md)

Faz live **sem fone**? O microfone pega o jogo e os amigos do Discord que saem na caixa de som, e a live
toca tudo duas vezes: uma pelo áudio do desktop e outra, atrasada e com eco, pelo microfone.

Apps de chat de voz resolvem isso com **cancelamento de eco acústico (AEC)**: tiram do microfone o que
está tocando na caixa. A fonte de microfone do OBS não faz isso. Este plugin cria uma fonte de áudio que
faz, **usando o cancelamento de eco que já vem no Windows**: sem app extra, sem cabo virtual, quase sem
CPU e sem exigir placa de vídeo.

## Como funciona

O Windows só liga o cancelamento de eco (Voice Clarity, ou os efeitos do driver de áudio) para quem abre
o microfone no modo **Comunicação**, o mesmo que o Discord e o Teams usam. O OBS abre no modo normal.
O plugin abre no modo Comunicação, fixa a referência do eco na saída que você escolher e impede o
Windows de abaixar os outros sons.

Medido no notebook do autor (array de microfones Intel Smart Sound, caixa Realtek, Windows 11 25H2):

| | AEC | nível do mic com uma voz tocando na caixa |
| --- | --- | --- |
| fonte de microfone do OBS (modo normal) | desligado | -35,7 dBFS |
| este plugin (modo Comunicação) | ligado | -90 dBFS (piso de ruído) |

A sua voz continua passando (com a supressão de ruído e o ganho automático do Windows).

## Requisitos

- Windows 11 22H2 ou mais novo (x64). Escolher a referência do eco exige o build 22621+.
- Um microfone para o qual o Windows (ou o driver) ofereça cancelamento de eco no modo Comunicação. A
  maioria dos notebooks com Windows 11 24H2+ tem o **Voice Clarity** da Microsoft; se não tiver, o log
  avisa (veja abaixo).
- OBS Studio 32 ou mais novo.

## Instalar

1. Baixe `obs-aec-mic-<versão>-windows-x64.zip` em [Releases](../../releases).
2. Feche o OBS, extraia o zip e rode o `install.bat`. Ele copia o plugin para
   `C:\ProgramData\obs-studio\plugins\aec-mic` (sobrevive à atualização do OBS). O `uninstall.bat` remove.
3. No OBS, adicione a fonte **"Microfone com cancelamento de eco (Windows)"**.
4. **Desative o microfone antigo** (Configurações → Áudio → Microfone/Áudio auxiliar = Desativado, ou
   remova a fonte de microfone das cenas). Senão a sua voz é capturada duas vezes.

O plugin também aparece em **Ferramentas → Gerenciador de Plugins**.

### Configurações

- **Microfone:** o dispositivo de comunicação padrão do Windows, ou um microfone específico.
- **Cancelar o som que sai em:** a saída cujo som deve sair do microfone. O jogo e o chat de voz precisam
  tocar nessa mesma saída; som em outro dispositivo (ex.: monitor pelo HDMI) não é cancelado.

### Dicas

- Configurações de Som do Windows → *Mais configurações de som* → aba **Comunicações** → **Não fazer
  nada**. O plugin já pede para não abaixar os outros sons, mas assim nada mais abaixa o jogo.
- Volume da caixa razoável: alto-falante de notebook distorcendo no máximo é mais difícil de cancelar.
- Deixe o Discord no microfone de sempre: ele tem o próprio cancelamento de eco.

## Está funcionando?

OBS → **Ajuda → Arquivos de log → Ver log atual**, procure:

```
[aec-mic] mic: <seu mic> | 48000 Hz, 1 ch | echo cancellation: ON
```

`not reported / off` quer dizer que o Windows não oferece cancelamento de eco para esse microfone.

## Compilar

Veja a seção *Build* do [README em inglês](README.md#build).

## Uso de IA

Este plugin foi escrito com ajuda de IA (Claude Code), sob a direção do autor. As medições de eco acima
foram feitas em hardware de verdade, o código passou por revisões independentes e o autor usa o plugin
nas próprias lives.

## Licença

GPL-2.0 ou posterior, como o OBS Studio. Veja [LICENSE](LICENSE).
