describe('Background-clip', () => {
  it('works with default value as border-box', async () => {
    const div = createElement(
      'div',
      {
        style: {},
      },
      [
        createElement('div', {
          style: {
            border: '5px solid rgba(0,0,0,0.3)',
            backgroundColor: '#f40',
            padding: '10px',
            backgroundRepeat: 'no-repeat',
            height: '50px',
            width: '100px',
          },
        }),
      ]
    );
    append(BODY, div);
    await snapshot();
  });

  it('works with border-box', async () => {
    const div = createElement(
      'div',
      {
        style: {},
      },
      [
        createElement('div', {
          style: {
            border: '5px solid rgba(0,0,0,0.3)',
            backgroundColor: '#f40',
            padding: '10px',
            backgroundRepeat: 'no-repeat',
            backgroundClip: 'border-box',
            height: '50px',
            width: '100px',
          },
        }),
      ]
    );
    append(BODY, div);
    await snapshot();
  });

  it('works with padding-box', async () => {
    const div = createElement(
      'div',
      {
        style: {},
      },
      [
        createElement('div', {
          style: {
            border: '5px solid rgba(0,0,0,0.3)',
            backgroundColor: '#f40',
            padding: '10px',
            backgroundRepeat: 'no-repeat',
            backgroundClip: 'padding-box',
            height: '50px',
            width: '100px',
          },
        }),
      ]
    );
    append(BODY, div);
    await snapshot();
  });

  it('works with content-box', async () => {
    const div = createElement(
      'div',
      {
        style: {},
      },
      [
        createElement('div', {
          style: {
            border: '5px solid rgba(0,0,0,0.3)',
            backgroundColor: '#f40',
            padding: '10px',
            backgroundRepeat: 'no-repeat',
            backgroundClip: 'content-box',
            height: '50px',
            width: '100px',
          },
        }),
      ]
    );
    append(BODY, div);
    await snapshot();
  });

  it('works with text', async () => {
    const p = createElement(
      'p',
      {
        style: {
          background: 'linear-gradient(60deg, red, yellow, red, yellow, red)',
          color: 'rgba(0, 0, 0, 0.5)',
          backgroundClip: 'text',
        },
      },
      [createText('The background is clipped to the foreground text.')]
    );
    append(BODY, p);
    await snapshot();
  });
});
