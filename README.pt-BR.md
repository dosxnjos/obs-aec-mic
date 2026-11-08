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

## Comparado a outras opções de cancelamento de eco

Cada opção segue um caminho diferente, e o caminho muda o que você precisa instalar e o que dá para ajustar. Os números dos outros projetos são os que os autores informam; nenhum foi testado aqui.

| | Usa a saída da caixa como referência? | Roda onde / o que instalar | Custo de CPU / placa de vídeo | Atraso a mais (declarado) | Ajustável? |
| --- | --- | --- | --- | --- | --- |
| **Este plugin** (modo Comunicação do Windows) | Sim, a saída que você escolher (o Windows escolhe se o microfone não deixar) | No motor de áudio do Windows, antes de o sinal chegar ao OBS; nada a mais | ~1% da CPU total num teste único e grosseiro no notebook do autor; sem placa de vídeo | Não medido | Só a saída de referência. O Windows também aplica os outros efeitos de voz dele (no notebook testado: beamforming, ganho automático, supressão de ruído), e o app não consegue desligá-los |
| **WebRTC AEC3** como VST3 ([AEC3-Echo-Canceller](https://github.com/RadegasTCZ/AEC3-Echo-Canceller)) ou como app + cabo virtual ([aec-client](https://github.com/samudinzul/aec-client) (AEC3 ou motores neurais, DTLN por padrão), [windows-aec-bridge](https://github.com/nVitius/windows-aec-bridge)) | Sim (loopback da caixa) | No OBS por um plugin que hospeda VST3 (atkAudio), ou num app à parte + cabo virtual (ex.: VB-CABLE) | Os projetos não publicam | ~30 ms informados ao host (VST3) e 30–40 ms (aec-client), segundo os autores | Sim, conforme o projeto |
| **Filtros neurais** ([LocalVQE](https://github.com/localai-org/LocalVQE), [VoiceAec](https://github.com/384615666/VoiceAec)) | Sim, uma fonte do OBS (normalmente o Áudio do desktop) | Filtro do OBS, na CPU por padrão; nada a mais (o VoiceAec também tem um app + cabo virtual opcional) | LocalVQE: ~3,2 ms a cada 16 ms de áudio com o modelo padrão e 4 threads, ou ~1,3 ms com o modelo só de eco v1.4-AEC em 1 thread (Ryzen 9 7900, segundo o README). VoiceAec: ~3–7% segundo o autor | 16 ms do algoritmo (LocalVQE, segundo o README); dezenas de ms (VoiceAec, segundo o autor) | Sim |
| **NVIDIA Broadcast** | Não. O FAQ da NVIDIA descreve o "Room Echo Removal" como remoção de "echo and reverb that typically occurs in small rooms", sem entrada de referência do alto-falante | App Broadcast (microfone virtual), ou o filtro Supressão de ruído do OBS (método NVIDIA) com o Audio Effects SDK da NVIDIA | Exige placa NVIDIA RTX (RTX 2060 ou mais nova) | Não informado | Não se aplica (sem referência da caixa) |
| **Noise gate / supressão de ruído** (noise gate e supressão de ruído do OBS: RNNoise, Speex) | Não | Filtro do OBS; nada a mais | Não comparado | Não comparado | Sim (limiares) |

Notas. O README do LocalVQE diz que o Windows está "implemented but unverified", e a versão publicada não traz executáveis (o VoiceAec empacota uma versão do filtro para Windows). O filtro do VoiceAec para o OBS é compilado com os headers do OBS 32.2.2, então exige o OBS 32.2 ou mais novo, segundo o README dele (o caminho pelo app + cabo virtual não depende da versão do OBS). O noise gate só fecha no silêncio, então o eco passa sempre que você fala. A supressão de ruído tira chiado e ventoinha, não voz.

### O que o cancelador do próprio Windows traz (quando o seu mic tem um)

Isso só vale se o log mostrar `echo cancellation: ON` para o seu microfone.

- **A referência vem de dentro do motor de áudio.** O próprio Windows mistura o que vai para a caixa, então o cancelador pega a referência ali, sem um segundo app nem cabo virtual no meio. Canceladores em app ou filtro capturam uma cópia por loopback e estimam o atraso por conta própria (o AEC3 faz isso sozinho, e alguns projetos também deixam ajustar).
- **É o cancelador que vem com o sistema.** É o Voice Clarity da Microsoft ou, em muitos notebooks, os efeitos do fabricante afinados para aquele microfone. Em arrays de microfones, ele trabalha junto com o beamforming, que foca na sua voz.
- **Nada a mais para instalar ou rotear.** Você adiciona uma fonte no OBS: sem app à parte, sem cabo virtual, sem placa de vídeo. A fonte acompanha o Windows quando o dispositivo padrão muda.
- **Leve no notebook do autor.** Num teste único e grosseiro, o processo de áudio do Windows (que atende todo o áudio do PC) foi de uns 28% para 42% de um núcleo, de 14 no total, ou seja, cerca de 1% do processador. Não dá para comparar com os números dos outros projetos, que vêm de outras máquinas.
- **Cancela de verdade, não é gate.** A voz da caixa caiu de -35,7 dBFS (modo Padrão) para o piso de ruído (-90 dBFS). Com a referência do eco apontada para outra saída, ela voltou (-27,9 dBFS). O Windows remove o que está tocando; não só emudece o microfone.

O modo Comunicação pode fazer o Windows abaixar os outros sons (ducking); a fonte desliga isso. Testado num notebook só (array Intel Smart Sound, Windows 11 25H2), com os níveis acima. Falar por cima da caixa (fala dupla) foi conferido só de ouvido; o atraso não foi medido.

### Quando outra opção é melhor

- **O log diz `not reported / off`.** O Windows não informou cancelamento de eco para esse microfone, então provavelmente não oferece (ou a sua versão do Windows não sabe dizer). Este plugin não consegue criar um. Confira de ouvido e, se o eco continuar, teste uma opção com AEC3 ou neural.
- **Você quer ajustar o cancelador.** Outras opções deixam ajustar o atraso do eco (AEC3-Echo-Canceller, windows-aec-bridge), escolher o motor ou o modelo (aec-client, LocalVQE, VoiceAec) ou o nível da supressão de ruído (AEC3-Echo-Canceller). Aqui só dá para escolher a saída de referência.
- **Você quer manter o seu próprio processamento de voz.** O modo Comunicação também liga os outros efeitos de voz do Windows, e o app não consegue desligá-los (no notebook testado: ganho automático, supressão de ruído e beamforming). Se você já tem uma cadeia de voz ajustada, uma opção que só cancela o eco combina mais.
- **Você não usa Windows 11 22H2 ou mais novo.** Este plugin exige. O VST3 do AEC3 e o aec-client também rodam no Windows 10, e o LocalVQE é testado no Linux.

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
